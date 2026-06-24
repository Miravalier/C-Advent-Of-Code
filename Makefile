.DEFAULT_GOAL = watch

.PHONY: help
help:
	@echo "make watch"
	@echo "  run main.c when a file changes"


.PHONY: watch
watch:
	@. .venv/bin/activate && python watcher.py


.PHONY: valgrind
valgrind:
	clang -I src src/*.c -lm -o test_executable -g
	valgrind --track-origins=yes --leak-check=full ./test_executable
	@rm -f test_executable
