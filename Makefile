count:
	@echo lines: `find . | grep -e \\\.[ch]| xargs cat | grep -v // | grep -v ^$$ | wc -l`