# vi: syntax=python:et:ts=4
from pkgconfig import run_pkg_config
from config_check_utils import find_include
from SCons.Util import AppendPath

from os.path import join

def CheckQtQuick(context):

    context.Message("Checking for QtQuick development files... ")

    env = context.env
    backup = env.Clone().Dictionary()

    if env.get("qtdir", ""):
        qtdir = env.get("qtdir", "")
        backup_pkg_config_path = env["ENV"]["PKG_CONFIG_PATH"]
        env["ENV"]["PKG_CONFIG_PATH"] = AppendPath(env.get("PKG_CONFIG_PATH",""), join(qtdir, "lib/pkgconfig"), sep=',')

    found = run_pkg_config(context, "Qt5Quick")

    if env.get("qtdir", ""):
        env["ENV"]["PKG_CONFIG_PATH"] = backup_pkg_config_path

    if found:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        env.Replace(**backup)
        return False

config_checks = { "CheckQtQuick" : CheckQtQuick }
