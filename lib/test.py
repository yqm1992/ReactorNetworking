import os
import commands

def exec_cmd(cmd):
    status, output = commands.getstatusoutput(cmd)
    return output

status, output = commands.getstatusoutput("ls *.c")
for file_name in exec_cmd("ls *.c").split():
    cmd = "mv %s %spp" %(file_name, file_name)
    exec_cmd(cmd)
    print cmd
