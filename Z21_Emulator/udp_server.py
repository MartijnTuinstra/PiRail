#python3 /usr/bin

import logging
import socket
import time
import random
import threading

# log = logging.getLogger('udp_server')

trains = {}
class train:
    def __init__(self, dcc):
        print("Create Train", dcc, flush=True)
        self.dcc = dcc
        self.speed = 0
        self.stufen = 4
        self.forward = False
        self.speed = 0
        self.doppel = False
        self.Smart = False
        self.F = [0 for i in range(0, 29)]

    def get_info(self):
        data = [0xEF, (self.dcc & 0x3F00) >> 8, self.dcc & 0xFF, self.stufen, (self.forward << 7) + (self.speed & 0x7F),
            (self.doppel << 6) + (self.Smart << 5) + (self.F[0] << 4) + (self.F[4] << 3) + (self.F[3] << 2) + (self.F[2] << 1) + self.F[1],
            (self.F[12] << 7) + (self.F[11] << 6) + (self.F[10] << 5) + (self.F[9] << 4) + (self.F[8] << 3) + (self.F[7] << 2) + (self.F[6] << 1) + self.F[5],
            (self.F[20] << 7) + (self.F[19] << 6) + (self.F[18] << 5) + (self.F[17] << 4) + (self.F[16] << 3) + (self.F[15] << 2) + (self.F[14] << 1) + self.F[13],
            (self.F[28] << 7) + (self.F[27] << 6) + (self.F[26] << 5) + (self.F[25] << 4) + (self.F[24] << 3) + (self.F[23] << 2) + (self.F[22] << 1) + self.F[21]]

        checksum = 0
        for cell in data:
            checksum ^= cell
        data = data + [checksum]
        data = [4+len(data), 0, 0x40, 0] + data
        return bytes(data)

    def set_speed(self, stufen, speed, forward):
        if(self.stufen == 4):
            if(speed < 0):
                speed = 0
            elif(speed > 128):
                speed = 128
        self.speed = speed
        self.forward = forward

    def set_func(self, type, func):
        print("F"+str(func)+" "+str(type), flush=True)
        if func > 29 or func < 0:
            return

        if type <= 1:
            self.F[func] = type
        elif type == 2:
            self.F[func] ^= 1


class udp_server:
    def __init__(self, host='127.0.0.1', port=1234):
        self.host = host
        self.port = port
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        # log.info("Listening on udp %s:%s" % (host, port))
        print("Listening on udp %s:%s" % (host, port), flush=True)
        self.s.bind((host, port))

    def read(self, size):
        (data, addr) = self.s.recvfrom(size)
        #log.debug("RX: %r" % (data,))
        return (data, addr)

    def write(self, data, addr):
        #log.debug("TX: %r"%(data,))
        return self.s.sendto(data, addr)

    def __del__(self):
        self.s.close()

class Z21_client:
    def __init__(self, Z21, addr):
        print("New Client", addr, flush=True)
        self.Z21 = Z21
        self.addr = addr
        self.flags = 0x0

    def __del__(self):
        print("Del client", flush=True)

    def set_broadcast(self, cast):
        print("Set Broadcast", cast, self.addr, flush=True)
        self.flags = int.from_bytes(cast, 'little')

    def get_broadcast(self):
        print("Get Broadcast", cast, self.addr, flush=True)
        return self.flags

class Z21_message:
    def __init__(self, Z21, message, addr):
        self.Z21 = Z21

        length = message[0] + (message[1] << 8)
        header = message[2] + (message[3] << 8)
        checksum = 0
        for i in range(4, length - 1):
            checksum ^= message[i]

        if header is 0x10:
            print("LAN_GET_SERIAL_NUMBER", flush=True)
            self.send(addr, self.SERIAL_NUMBER)
        elif header is 0x30:
            print("LAN_LOGOFF", flush=True)
            Z21.clients.remove(addr)
        elif header is 0x40:
            if(checksum != message[length-1]):
                print("Z21 wrong checksum", flush=True)
                print(message, flush=True)
                return

            XHeader = message[4]
            if(XHeader == 0x21): # LAN_X_GET_VERSION
                if(message[5] == 0x21):
                    print("LAN_X_GET_VERSION", flush=True)
                elif(message[5] == 0x24):
                    print("LAN_X_GET_STATUS", flush=True)
                    self.send(addr, self.X_STATUS_CHANGED, Z21=Z21)
                elif(message[5] == 0x80):
                    print("LAN_X_SET_TRACK_POWER_OFF", flush=True)
                    Z21.track_power_off = True
                    self.send_all(self.X_BC_TRACK_POWER_OFF)
                elif(message[5] == 0x81):
                    print("LAN_X_SET_TRACK_POWER_ON", flush=True)
                    Z21.track_power_off = False
                    self.send_all(self.X_BC_TRACK_POWER_ON)

            elif(XHeader == 0x80): # LAN_X_BC_STOPPED
                print("LAN_X_SET_STOP", flush=True)
            elif(XHeader == 0xF1): # LAN_X_GET_FIRMWARE_VERSION
                print("LAN_X_GET_FIRMWARE_VERSION", flush=True)
                self.send(addr, self.FIRWARE_VERSION)
            elif(XHeader == 0x43): # LAN_X_GET_TURNOUT_INFO
                print("LAN_X_GET_TURNOUT_INFO", flush=True)
            elif(XHeader == 0xE3): # LAN_X_GET_LOCO_INFO
                print("LAN_X_GET_LOCO_INFO", flush=True)
                dcc = ((message[6] & 0x3F) << 8) + message[7]
                if dcc not in trains:
                    trains[dcc] = train(dcc)

                self.send(addr, self.LAN_X_LOCO_INFO, train=dcc)

                # self.send_all(trains[dcc].get_info())
            elif(XHeader == 0xE4): # LAN_X_SET_LOCO_DRIVE
                if(message[5] & 0xF0 == 0x10):
                    dcc = ((message[6] & 0x3F) << 8) + message[7]
                    #try:
                    if dcc in trains:
                        trains[dcc].set_speed(message[5] & 0x3, message[8] & 0x7F, message[8] >> 7)
                        print("LAN_X_SET_LOCO_DRIVE", dcc, message[8] & 0x7F, message[8] >> 7, flush=True)
                        # Z21.server.write(trains[dcc].get_info(), addr)
                    else:
                        trains[dcc] = train(dcc)

                    self.send_all(self.LAN_X_LOCO_INFO, train=dcc)
                    #except Exception as e:
                    #   print("Exception", e, flush=True)
                elif(message[5] & 0xF0 == 0xF0):
                    print("LAN_X_SET_LOCO_FUNC", flush=True)
                    dcc = ((message[6] & 0x3F) << 8) + message[7]

                    if dcc in trains:
                        trains[dcc].set_func((message[8] & 0xC0) >> 6, message[8] & 0x3F)
                        # Z21.server.write(trains[dcc].get_info(), addr)
                    else:
                        trains[dcc] = train(dcc)

                    self.send_all(self.LAN_X_LOCO_INFO, train=dcc)
            else:
                print("Unknown", message, flush=True)
        elif header is 0x50: #LAN_GET_BROADCASTFLAGS
            print("LAN_SET_BROADCASTFLAGS", flush=True)
            for c in Z21.clients:
                if c.addr == addr:
                    c.set_broadcast(message[4:8])
        elif header is 0x51: #LAN_GET_BROADCASTFLAGS
            print("LAN_GET_BROADCASTFLAGS", flush=True)
            self.send(addr, self.LAN_GET_BROADCASTFLAGS, flags=[c.flags for c in Z21.clients if c.addr == addr][0])
        elif header is 0x85: #LAN_GET_HWINFO
            print("LAN_SYSETMSTATE_GETDATA", flush=True)
            self.send(addr, self.SYSTEMSTATE_DATACHANGED, Z21=Z21)
        elif header is 0x1A: #LAN_GET_HWINFO
            print("LAN_GET_HWINFO", flush=True)
            self.send(addr, self.LAN_GET_HWINFO)
        elif header is 0x60: #LAN_GET_LOCOMODE
            print("LAN_GET_LOCOMODE", flush=True)
            self.send(addr, self.LAN_GET_LOCOMODE, data1=int(message[4]), data2=int(message[5]))
        elif header is 0x61: #LAN_GET_LOCOMODE
            print("LAN_SET_LOCOMODE", flush=True)
        elif header is 0x70: #LAN_GET_TURNOUTMODE
            print("LAN_GET_TURNOUTMODE", flush=True)
        elif header is 0x71: #LAN_GET_TURNOUTMODE
            print("LAN_SET_TURNOUTMODE", flush=True)
        else:
            print("Unknown", message, flush=True)


    def LAN_X_LOCO_INFO(self, train=None):
        return trains[train].get_info()

    def SYSTEMSTATE_DATACHANGED(self, Z21=None):
        data = b"\x14\x00\x84\x00"

        data += Z21.mainCurrent.to_bytes(2, byteorder='little', signed=True)
        data += Z21.progCurrent.to_bytes(2, byteorder='little', signed=True)
        data += Z21.FmainCurrent.to_bytes(2, byteorder='little', signed=True)
        data += Z21.temp.to_bytes(2, byteorder='little', signed=True)
        data += Z21.vccsup.to_bytes(2, byteorder='little')
        data += Z21.vcc.to_bytes(2, byteorder='little')

        data += (Z21.emergency_stop * 0x01 + Z21.track_power_off * 0x02 + Z21.short * 0x04 + Z21.prog_mode * 0x20).to_bytes(1, byteorder='little')
        data += b"\x00\x00\x00"

        return data


    def SERIAL_NUMBER(self):
        return b"\x08\x00\x10\x00\x12\x34\x56\x78"

    def unkon_command(self):
        return b"\x07\x00\x40\x00\x61\x82\xe3"

    def BC_TRACK_OFF(self):
        return b"\x07\x00\x40\x00\x61\x00\x61"

    def BC_TRACK_ON(self):
        return b"\x07\x00\x40\x00\x61\x01\x60"

    def BC_SHORT(self):
        return b"\x07\x00\x40\x00\x61\x08\x69"

    def BC_PROGRAMMING(self):
        return b"\x07\x00\x40\x00\x61\x02\x63"

    def FIRWARE_VERSION(self):
        return b"\x09\x00\x40\x00\xF3\x0A\x01\x23\xDB"

    def LAN_GET_BROADCASTFLAGS(self, flags=0):
        return (b"\x08\x00\x51\x00" + flags.to_bytes(4, 'little'))

    def LAN_GET_HWINFO(self):
        return b"\x0C\x00\x1A\x00" + b"\x00\x00\x02\x00" + b"\x20\x01\x00\x00"

    def LAN_GET_LOCOMODE(self, data1=0, data2=0):
        return b"\x07\x00\x60\x00"+data1.to_bytes(1, 'little')+data2.to_bytes(1, 'little')+b"\x00"+(data1^data2).to_bytes(1,'little')

    def X_STATUS_CHANGED(self, Z21=None):
        data = b"\x08\x00\x40\x00\x62\x22"
        byte = Z21.emergency_stop + (Z21.track_power_off << 1) + (Z21.short << 2) + (Z21.prog_mode << 5)
        data += byte.to_bytes(1, 'little') + (0x62 ^ 0x22 ^ byte).to_bytes(1, 'little')
        return data

    def X_BC_TRACK_POWER_OFF(self):
        return b"\x07\x00\x40\x00\x61\x00\x61"
    def X_BC_TRACK_POWER_ON(self):
        return b"\x07\x00\x40\x00\x21\x81\xa0"

    def send(self, addr, func, **kwargs):
        data = func(**kwargs)
        print("send "+str(data))
        self.Z21.server.write(data, addr)

    def send_all(self, func, **kwargs):
        data = func(**kwargs)

        print("send all "+str(data))

        for client in self.Z21.clients:
            self.Z21.server.write(data, client.addr)


class Z21:
    def __init__(self, host='127.0.0.1', port=21105, server=None):
        if(server is None):
            self.server = udp_server(host, port)
        else:
            self.server = server
        self.clients = []

        self.stop = False

        self.track_power_off = False
        self.emergency_stop = False
        self.short = False
        self.prog_mode = False

        self.mainCurrent = 0
        self.progCurrent = 0
        self.FmainCurrent = 0
        self.temp = 0
        self.vccsup = 0
        self.vcc = 0


    def run(self):
        print("run", flush=True )
        (data, addr) = self.read()

    def start_broadcast(self):
        self.stop = False
        self.thread = threading.Thread(target=self.broadcast)
        self.thread.start()

    def stop_broadcast(self):
        self.stop = True
        self.thread.join()

    def broadcast(self):
        while not self.stop:
            self.mainCurrent = random.randrange(-1000,1000,1)
            self.progCurrent = random.randrange(-1000,1000,1)
            self.FmainCurrent = random.randrange(-1000,1000,1)
            self.temp = random.randrange(21,30,1)
            self.vccsup = random.randrange(16000,19000,1)
            self.vcc = random.randrange(15500,19500,1)

            data = Z21_message.SYSTEMSTATE_DATACHANGED(None, Z21=self)

            for client in self.clients:
                if client.flags & 0x100:
                    self.server.write(data, client.addr)

            time.sleep(10)

    def read(self, data=None, addr=None):
        if data is None:
            (data, addr) = self.server.read(1024)

        if addr not in [c.addr for c in self.clients]:
            self.clients.append(Z21_client(self, addr))

        Z21_message(self, data, addr)

        length = data[0] + (data[1] << 8)
        if(length != len(data)):
            self.read(data[length:], addr)

        return (data, addr)

    def __del__(self):
        del self.server

        if not self.stop:
            self.stop = False
            self.thread.join()


if __name__ == "__main__":
    # FORMAT_CONS = '%(asctime)s %(name)-12s %(levelname)8s\t%(message)s'
    # logging.basicConfig(level=logging.DEBUG, format=FORMAT_CONS)

    server = udp_server("0.0.0.0", 21105)
    z21 = Z21(server=server)

    z21.start_broadcast()

    try:
        while True:
            z21.run()
            time.sleep(0.001)
    except KeyboardInterrupt as e:
        pass

    print("Stop")
    z21.stop_broadcast()
