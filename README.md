This is i8080 cross assembler that produces binary files in RKS format.

Structure of RKS file is simple:
+0: Start address (little endian)
+2: Last address  (little endian)
+4: binary data
+N: one byte of checksum at end of file

This assembler is somewhat compatible with the MIKRON assembler from Radio-86RK home computer.
Look at examples directory to get syntax details.
