# Build All Makefiles

linux: common-linux sysc-linux stat-linux assembler-linux compiler-linux interpreter-linux
cygwin: common-cygwin sysc-cygwin stat-cygwin assembler-cygwin compiler-cygwin interpreter-cygwin
clean: common-clean sysc-clean stat-clean assembler-clean compiler-clean interpreter-clean

.PHONY: common sysc stat assembler compiler interpreter common-clean sysc-clean stat-clean assembler-clean compiler-clean interpreter-clean 

assembler-linux: common-linux
	$(MAKE) -C ./assembler build-linux

common-linux:
	$(MAKE) -C ./common build-linux

compiler-linux: common-linux
	$(MAKE) -C ./compiler build-linux

interpreter-linux: common-linux sysc-linux
	$(MAKE) -C ./interpreter build-linux

stat-linux: common-linux
	$(MAKE) -C ./stat build-linux

sysc-linux:
	$(MAKE) -C ./sysc build-linux

assembler-cygwin: common-cygwin
	$(MAKE) -C ./assembler build-cygwin

common-cygwin:
	$(MAKE) -C ./common build-cygwin

compiler-cygwin: common-cygwin
	$(MAKE) -C ./compiler build-cygwin

interpreter-cygwin: common-cygwin sysc-cygwin
	$(MAKE) -C ./interpreter build-cygwin

stat-cygwin: common-cygwin
	$(MAKE) -C ./stat build-cygwin

sysc-cygwin:
	$(MAKE) -C ./sysc build-cygwin

assembler-clean:
	$(MAKE) -C ./assembler clean

common-clean:
	$(MAKE) -C ./common clean

compiler-clean:
	$(MAKE) -C ./compiler clean

interpreter-clean:
	$(MAKE) -C ./interpreter clean

stat-clean:
	$(MAKE) -C ./stat clean

sysc-clean:
	$(MAKE) -C ./sysc clean
