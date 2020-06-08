# MPI-IO Test (a.k.a. `fs_test`)

Although there are a host of existing file system and I/O test programs
available, most are not designed with parallel I/O in mind and are not useful
at the scale of the clusters at Los Alamos National Lab (LANL). LANL's MPI-IO
Test was written with parallel I/O and scale in mind.

The MPI-IO test is built on top of MPI's I/O calls and is used to gather timing
information for reading from and writing to file(s) using a variety of I/O
profiles; N processes writing to N files, N processes writing to one file, N
processes sending data to M processes writing to M files, or N processes
sending data to M processes to one file. A data aggregation capability is
available and the user can pass down MPI-IO, ROMIO and file system specific
hints.

The MPI-IO Test can be used for performance benchmarking and, in some cases, to
diagnose problems with file systems or I/O networks.  The newer versions of
MPI-IO test have been renamed to `fs_test`.  The most current version of
`fs_test` moved to Sourceforge but we cannot seem to find it any longer.

The MPI-IO Test (fs_test) is open sourced under LA-CC-05-013.
