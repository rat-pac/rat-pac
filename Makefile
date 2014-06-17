all:
	scons
	@echo 'You can run "scons" directly to compile RAT.'

installdata:
	scons installdata
	@echo 'You can run "scons installdata" directly to copy neutron files.'

doc:
	scons doc
	@echo 'You can run "scons doc" directly to extract/generate docs.'

clean:
	scons -c
	@echo 'You can run "scons -c" directly to clean build.'

.PHONY: doc clean installdata all

