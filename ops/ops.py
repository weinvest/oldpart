import os
import sys
import cmd

import libops as ops

class OPSCmd(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
        ops.initialize()
        ops.listen(ip, port)
        ops.run()

        self.machines = []

    def do_mas(self, machines):
        self.work_machines = [mac for mac in machines if mac in ops.allMachines()]

    def do_ls(self, fpath):
        ls_cmd = ops.ls_cmd(fpath)
        ops.send(ls_cmd)


    def do_cat(self, fpath):

        
