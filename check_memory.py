import os
import time
import sys
class PmapDecoder:
    def __init__(self) -> None:
        self.__config = {
            "filed length":[16, 8, 8, 8, 6, -1],
            "filed names":["Address", "Kbytes", "RSS", "Dirty", "Mode", "Mapping"]
        }
        self._values = []
    def get(self, pid):
        res = os.popen("pmap -x -q {}".format(pid))
        for line in res:
            line = line.strip("\n")
            value = {}
            start_pos = 0
            for index, name in enumerate(self.__config["filed names"]):
                length = self.__config["filed length"][index]
                if length == -1:
                    value[name] = line[start_pos:].strip()
                    break
                else:
                    value[name] = line[start_pos:start_pos + length].strip()
                    start_pos = start_pos + length
            self._values.append(value)

    def for_each(self, function):
        for value in self._values:
            function(value)

def print_values(value):
    if value["Mapping"] == "[ anon ]" and value["Mode"].startswith("rw"):
        command = "./pmem {} {} {}|grep '>'|grep -v 'v8::'|grep -v'node::'|grep -v 'v8_inspector::'".format(sys.argv[1], value["Address"], value["Kbytes"])
        res = os.popen(command)
        for line in res:
            print(line)
        time.sleep(1)
decoder = PmapDecoder()
decoder.get(sys.argv[1])
decoder.for_each(print_values)
