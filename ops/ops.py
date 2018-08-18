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

    def do_ls(self, l):
        pass
        
