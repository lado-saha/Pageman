savedcmd_/home/sih/linux-6.5.3/rootkit/pageman.mod := printf '%s\n'   pageman.o | awk '!x[$$0]++ { print("/home/sih/linux-6.5.3/rootkit/"$$0) }' > /home/sih/linux-6.5.3/rootkit/pageman.mod
