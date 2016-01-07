all: mkfs ls cat tee cp dump mkdir

cp: cp.cc Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o ProcessContext.o FileSystem.o Stat.o BitBlock.o Block.o SuperBlock.o
	g++ cp.cc Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o ProcessContext.o FileSystem.o Stat.o BitBlock.o Block.o SuperBlock.o -o cp

tee: tee.cc Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o ProcessContext.o FileSystem.o Stat.o BitBlock.o Block.o SuperBlock.o
	g++ tee.cc Kernel.o IndexNode.o FileDescriptor.o DirectoryEntry.o ProcessContext.o FileSystem.o Stat.o BitBlock.o Block.o SuperBlock.o -o tee

ls: ls.cc Stat.o DirectoryEntry.o Kernel.o FileSystem.o FileDescriptor.o ProcessContext.o
	g++ ls.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o -o ls

mkdir: mkdir.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o
	g++ mkdir.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o -o mkdir

cat: cat.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o
	g++ cat.cc Stat.o FileDescriptor.o FileSystem.o BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o ProcessContext.o Kernel.o -o cat

mkfs: mkfs.cc BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o
	g++ mkfs.cc BitBlock.o IndexNode.o DirectoryEntry.o Block.o SuperBlock.o -o mkfs

dump: dump.cc
	g++ dump.cc -o dump

Kernel.o: Kernel.cc
	g++ -c Kernel.cc

ProcessContext.o: ProcessContext.cc
	g++ -c ProcessContext.cc 

FileDescriptor.o: FileDescriptor.cc
	g++ -c FileDescriptor.cc
	
FileSystem.o: FileSystem.cc 
	g++ -c FileSystem.cc 

Stat.o: Stat.cc
	g++ -c Stat.cc

DirectoryEntry.o: DirectoryEntry.cc
	g++ -c DirectoryEntry.cc

BitBlock.o: BitBlock.cc
	g++ -c BitBlock.cc

IndexNode.o: IndexNode.cc
	g++ -c IndexNode.cc 

Block.o: Block.cc
	g++ -c Block.cc

SuperBlock.o: SuperBlock.cc	
	g++ -c SuperBlock.cc

clean:
	rm *.o mkfs ls cat tee cp dump mkdir

