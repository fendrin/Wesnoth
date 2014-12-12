# vi: syntax=python:et:ts=4
from pkgconfig import run_pkg_config
from config_check_utils import find_include
from SCons.Util import AppendPath

from os.path import join

def CheckQtPkg(context, env, name, qtdir):
    str=" "
    seq=("Checking for", name, "development files... ")
    context.Message(str.join(seq))

    if qtdir:
        backup_pkg_config_path = env["ENV"]["PKG_CONFIG_PATH"]
        env["ENV"]["PKG_CONFIG_PATH"] = AppendPath(env.get("PKG_CONFIG_PATH",""), join(qtdir, "lib/pkgconfig"), sep=',')

    found = run_pkg_config(context, name)

    if qtdir:
        env["ENV"]["PKG_CONFIG_PATH"] = backup_pkg_config_path

    if found:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False


def CheckQtQuick(context):

    env = context.env
    backup = env.Clone().Dictionary()

    qtdir = env.get("qtdir", "")

    found = CheckQtPkg(context, env, "Qt5Core", qtdir) & \
        CheckQtPkg(context, env, "Qt5Quick", qtdir) & \
        CheckQtPkg(context, env, "Qt5Qml", qtdir)

    if not found:
        env.Replace(**backup)
        return False

    backup_cppflags = env.get("CPPFLAGS", "")
    env.Append(CPPFLAGS = "-fPIC")

    test_program = """
        #include <QtCore/QtGlobal>\n
        #if QT_VERSION < 0x050300\n
        #error "qt version is too old! You should have 5.3.0 at least."\n
        #endif """

    test_program += """
        int main()
        {
        }
        \n"""

    context.Message("Checking for Qt version >= 5.3.0 ... ")

    if context.TryLink(test_program, ".cpp"):
        context.Result("yes")
    else:
        context.Result("no")
        env.Replace(**backup)
        return False

    test_program = """
        #include <QtCore/QString>\n
        #include <QtCore/QObject>\n"""

    test_program += """
        int main()
        {
        }
        \n"""

    context.Message("Checking for Qt core headers ... ")

    if context.TryLink(test_program, ".cpp"):
        context.Result("yes")
    else:
        context.Result("no")
        env.Replace(**backup)
        return False

    test_program = """
        #include <QtQuick/QQuickView>\n
        #include <QtQuick/QQuickItem>\n"""

    test_program += """
        int main()
        {
        }
        \n"""

    context.Message("Checking for Qt quick headers ... ")

    if context.TryLink(test_program, ".cpp"):
        context.Result("yes")
        env["CPPFLAGS"] = backup_cppflags
        return True
    else:
        context.Result("no")
        env.Replace(**backup)
        return False


config_checks = { "CheckQtQuick" : CheckQtQuick }
