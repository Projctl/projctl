#!/bin/fish

paru -Syu --needed just cmake clang
git pull
just build
sudo install -m 755 build/projctl /usr/local/bin/projctl
echo "Also it's nice to alias / make func 'p' as 'cd (projctl path \$argv[1])'"
