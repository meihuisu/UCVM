##
#    Handles the command list that can be executed (i.e. added
#    to the workflow queue). This specifies the command list
#    and also acts as the "router", recording the command execution
#    attempt to the log.

import sqlite3

def run_command(str, params):
    return