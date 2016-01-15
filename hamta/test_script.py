#!/usr/bin/python3

import hamta

h = hamta.Hamt()
n = 10000

for x in range(n): h[x] = x
for x in range(n): i = h[x]
for x in range(n): del h[x]
del x
