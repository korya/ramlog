MODULE=../ramlog.ko

test: $(MODULE)
	@echo "Running tests..."
	@for i in *.test.sh; do \
	  ./run-single-test.sh $(MODULE) $$i || exit 1; \
	done
	@echo "All test passed!"
