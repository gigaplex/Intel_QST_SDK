OS=$(shell uname -o)

.PHONY: all
all:
	if [ "$(OS)" = "Solaris" ]; then \
		make --directory src/Libraries/Solaris install; \
	elif [ "$(OS)" = "GNU/Linux" ]; then \
		make --directory src/Libraries/Linux install; \
	fi
	make --directory src/Programs/BusTest
	make --directory src/Programs/InstTest
	make --directory src/Programs/StatTest


