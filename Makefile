.PHONY: install-dev verify test docs-check clean

PYTHON ?= python3
PIP ?= pip3

install-dev:
	$(PIP) install -q -r requirements-dev.txt

verify: docs-check test
	$(PYTHON) tools/verify_repo.py

test:
	PYTHONPATH=. $(PYTHON) -m pytest modules tests -q

docs-check:
	@test -f docs/TEACHING_GAPS.md
	@test -f docs/COVERAGE.md
	@test -f docs/RESOURCE_POLICY.md
	@test -f docs/LEARNING_PATH.md
	@test -f sources/catalog.toml
	@test -f LICENSE

clean:
	find . -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null || true
