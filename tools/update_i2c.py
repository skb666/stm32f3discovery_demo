#!/usr/bin/python
import sys

def usage():
    print("Usage:")
    print(f" python {sys.argv[0]} [-d|--device <DEV>]  [-b|--baudrate <BAUD>] [-f|--file <FILE>] [-s|--slave <SLAVE_ADDR>] [-p|--bld]")

if __name__ == "__main__":
    import os
    import getopt
    import struct
    import time
    from mySerial import *
    from crc import CRC

    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:b:f:s:p", ["device=", "baudrate=", "file=", "--slave", "bld"])
    except getopt.GetoptError as err:
        print(err)
        usage()
        exit(2)

    i2c_slave = 0x0033
    partition_type_app = struct.pack('>I', 0xffffffff)
    partition_type_bld = struct.pack('>I', 0xa5a5a5a5)
    partition_type = partition_type_app

    portname = "COM5"
    baudrate = 115200
    filename = ""

    for op, val in opts:
        if op in ("-d", "--device"):
            portname = val
            print("get -d: %s" % portname)
        elif op in ("-b", "--baudrate"):
            baudrate = int(val)
            print("get -b: %d" % baudrate)
        elif op in ("-f", "--file"):
            filename = val
            print("get -f: %s" % filename)
        elif op in ("-s", "--slave"):
            i2c_slave = int(val, 16)
            print("get -s: 0x%04x" % i2c_slave)
        elif op in ("-p", "--bld"):
            partition_type = partition_type_bld
            print("get -p: update bld")
        else:
            assert False, "UNHANDLED OPTION"

    if not filename:
        if partition_type is partition_type_app:
            filename = "../app/emStudio/Output/Debug/Exe/stm32f3-app.bin"
            # filename = "../app/build/stm32f3-app.bin"
        else:
            filename = "../bld/emStudio/Output/Debug/Exe/stm32f3-bld.bin"
            # filename = "../bld/build/stm32f3-bld.bin"

    ports = SerialCommand.get_all_ports()
    if not portname in ports:
        print(ports)
        exit()

    sercomm = SerialCommand(portname, baudrate)
    if sercomm.openserial() < 0:
        print(f"Can't Open `{portname}`")
        exit()
    sercomm.used_port_info()

    i2c_dev_addr = struct.pack('>H', i2c_slave)
    i2c_reg_system_ctrl = struct.pack('>H', 0xff00)
    i2c_reg_update_data = struct.pack('>H', 0xff01)
    i2c_reg_update_status = struct.pack('>H', 0xff02)
    frame_head = b'\x55\xaa'
    frame_type_i2c_write = b'\xe1'
    frame_type_i2c_read = b'\xe2'
    pkg_type_init = struct.pack('>H', 0x1000)
    pkg_type_finish = struct.pack('>H', 0x0ffe)
    pkg_type_head= struct.pack('>H', 0x0001)
    pkg_type_data = struct.pack('>H', 0x0002)
    system_ctrl_reboot = struct.pack('>H', 0x0001)
    system_ctrl_boot_app = struct.pack('>H', 0x0002)
    system_ctrl_update_start = struct.pack('>H', 0x0003)

    def get_update_status():
        data = i2c_dev_addr + i2c_reg_update_status + struct.pack('>H', 0x0002)
        packed = frame_head + frame_type_i2c_read + struct.pack('>H', len(data)) + data
        sercomm.write_raw(packed)
        recv = sercomm.read_raw(2)
        status = struct.unpack('>H', recv)[0]
        return (status >> 13, (status >> 12) & 0x0001, status & 0x0fff)

    def update_init():
        data = i2c_dev_addr + i2c_reg_update_data + pkg_type_init
        packed = frame_head + frame_type_i2c_write + struct.pack('>H', len(data)) + data
        sercomm.write_raw(packed)

    def update_head(partition):
        crc32_mpeg2 = CRC("crc32_mpeg2")
        file_size_real = os.path.getsize(filename)
        data_size_one = 1024
        if file_size_real % data_size_one: 
            pkg_num_total = file_size_real // data_size_one + 1 
        else:
            pkg_num_total = file_size_real // data_size_one
        with open(filename, 'rb') as f_obj:
            crc32_mpeg2.reset()
            while True:
                data = f_obj.read(data_size_one)
                if not data:
                    break
                crc32_mpeg2.accumulate(data)
            file_crc = crc32_mpeg2.get()
        data = i2c_dev_addr + i2c_reg_update_data + pkg_type_head + partition + struct.pack('>IIHH', file_crc, file_size_real, data_size_one, pkg_num_total)
        packed = frame_head + frame_type_i2c_write + struct.pack('>H', len(data)) + data
        sercomm.write_raw(packed)
        return (file_crc, file_size_real, data_size_one, pkg_num_total)
    
    def update_data(file_crc, file_size_real, data_size_one, pkg_num_total):
        crc32_mpeg2_file = CRC("crc32_mpeg2")
        crc32_mpeg2_pkg = CRC("crc32_mpeg2")
        pkg_data_seek = 0
        pkg_num_send = 1
        crc32_mpeg2_file.reset()
        crc32_mpeg2_pkg.reset()
        retry = 3
        while retry and (pkg_data_seek < file_size_real and pkg_num_send <= pkg_num_total):
            retry -= 1
            with open(filename, 'rb') as f_obj:
                f_obj.seek(pkg_data_seek, 0)
                update_data = f_obj.read(data_size_one)
                data_len = len(update_data)
                pkg_crc = crc32_mpeg2_pkg(update_data)
            data = i2c_dev_addr + i2c_reg_update_data + pkg_type_data + struct.pack('>IHH', pkg_crc, pkg_num_send, data_len) + update_data
            packed = frame_head + frame_type_i2c_write + struct.pack('>H', len(data)) + data
            print(f"pkg_num: 0x{pkg_num_send:03x}, pkg_crc: 0x{pkg_crc:08x}")
            sercomm.write_raw(packed)
            time.sleep(1)
            get_status_retry = 3
            while get_status_retry:
                get_status_retry -= 1
                (errno, running, pkg_num) = get_update_status()
                time.sleep(0.2)
                if errno != 7:
                    break
            print(f"status: {errno}, {running}, 0x{pkg_num:03x}\r\n")
            if pkg_num == pkg_num_send:
                retry = 3
                crc32_mpeg2_file.accumulate(update_data)
                pkg_num_send += 1
                pkg_data_seek += data_len
        print(f"file_crc: 0x{crc32_mpeg2_file.get():08x}")
        if file_size_real != pkg_data_seek or file_crc != crc32_mpeg2_file.get():
            return False
        return True
    
    def update_finish():
        data = i2c_dev_addr + i2c_reg_update_data + pkg_type_finish
        packed = frame_head + frame_type_i2c_write + struct.pack('>H', len(data)) + data
        sercomm.write_raw(packed)

    def system_ctrl(ctrl_flag):
        data = i2c_dev_addr + i2c_reg_system_ctrl + ctrl_flag
        packed = frame_head + frame_type_i2c_write + struct.pack('>H', len(data)) + data
        sercomm.write_raw(packed)
    
    print("="*50)
    packed = frame_head + struct.pack('>H', 0xfffe)[:1]
    chars = sercomm.write_raw(packed)
    print("set to big-endian\r\n")
    time.sleep(0.2)

    """ 获取初始状态 """
    (errno, running, pkg_num) = get_update_status()
    print(f"status: {errno}, {running}, 0x{pkg_num:03x}\r\n")
    time.sleep(0.2)

    """ 升级初始化 """
    retry = 3
    while retry and (errno or not running or pkg_num != 0x0fff):
        retry -= 1
        update_init()
        time.sleep(3)
        (errno, running, pkg_num) = get_update_status()
        time.sleep(0.2)
    print(f"status: {errno}, {running}, 0x{pkg_num:03x}\r\n")
    if not retry and (errno or not running or pkg_num != 0x0fff):
        print("error: update_init")
        exit()

    """ 升级文件头信息包 """
    retry = 3
    while retry and (errno or not running or pkg_num != 0x0000):
        retry -= 1
        (file_crc, file_size_real, data_size_one, pkg_num_total) = update_head(partition_type)
        time.sleep(0.2)
        (errno, running, pkg_num) = get_update_status()
        time.sleep(0.2)
    print(f"file_crc: 0x{file_crc:08x}\r\nfile_size_real: {file_size_real}\r\ndata_size_one: {data_size_one}\r\npkg_num_total: {pkg_num_total}")
    print(f"status: {errno}, {running}, 0x{pkg_num:03x}\r\n")
    if not retry and (errno or not running or pkg_num != 0x0000):
        print("error: update_head")
        exit()

    """ 升级文件数据包 """
    isok = update_data(file_crc, file_size_real, data_size_one, pkg_num_total)
    time.sleep(1)
    if not isok:
        print("error: update_data")
        exit()

    """ 结束升级 """
    update_finish()
    time.sleep(3)
    (errno, running, pkg_num) = get_update_status()
    time.sleep(0.2)
    print(f"status: {errno}, {running}, 0x{pkg_num:03x}\r\n")
    if not errno and not running and pkg_num == 0x0ffe:
        print("success")
    else:
        print("failed")

    """ 重启设备 """
    system_ctrl(system_ctrl_reboot)

    print("="*50)
    sr = SerialReceive(sercomm)
    sr.start()

    while True:
        sdata = input()
        if sdata == "q!":
            break

    sercomm.closeserial()
