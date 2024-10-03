#"/usr/bin/g++",
#            "args": [
#                "-fdiagnostics-color=always",
#                "-std=gnu++23",
#                "-g",
#                "${file}",
#                "-o",
#                "${fileDirname}/${fileBasenameNoExtension}",
#                "-Wall",
#                "-O3"
tttesg: tttesg.cpp
	g++ -fdiagnostics-color=always -std=gnu++23 -g $< -o $@ -Wall -O3