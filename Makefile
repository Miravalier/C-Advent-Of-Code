.DEFAULT_GOAL = watch

.PHONY: help
help:
	@echo "make watch"
	@echo "  run main.c when a file changes"


.PHONY: watch
watch:
	@. .venv/bin/activate && python watcher.py
