MODULE=../ramlog.ko

test: $(MODULE)
	@echo "Running tests..."
	@for i in *.test.sh; do \
	  ./run-single-test.sh $(MODULE) $$i || exit 1; \
	done
	@echo "All test passed!"

crash_test: $(MODULE)
	@echo "Running crash tests..."
	@./run-single-test.sh $(MODULE) crash-test.sh
	@echo "All test passed!"
