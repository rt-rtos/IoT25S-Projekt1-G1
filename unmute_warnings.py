# The renesas-ra Arduino builder compiles everything with -w, which mutes
# every warning including the -Wall -Wextra in build_flags. Strip it for
# project code (src/ and lib/) only. The core's own -Wconversion and
# -Wshadow fire inside its headers, and -Waggregate-return is obsolete;
# drop those three as well so the build reports our code, not Arduino's.
Import("env", "projenv")

DROP = "-w -Waggregate-return -Wconversion -Wshadow"

projenv.ProcessUnFlags(DROP)
for lb in env.GetLibBuilders():
    if lb.path.startswith(env.subst("$PROJECT_LIBDEPS_DIR")):
        continue
    if lb.path.startswith(env.subst("$PROJECT_DIR")):
        lb.env.ProcessUnFlags(DROP)
