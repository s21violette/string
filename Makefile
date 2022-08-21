WWW = -Wall -Wextra -Werror

all: clean test

re: clean all 

test: string.a clean_obj
	gcc $(WWW) -l check unit_tests.c string.a -o test
	./test

gcov_report: gcov_report_build
	rm -rf *.gcda
	rm -rf *.gcno
	rm -rf *.info

gcov_report_build:
	gcc $(WWW) --coverage -l check unit_tests.c sprintf.c string.c -o gcov_report
	./gcov_report
	lcov -t "./gcov_report" -o gcov_report.info -c -d .
	genhtml -o report gcov_report.info

string.a: build_string clean_obj

build_string:
	gcc $(WWW) -c sprintf.c string.c
	ar rc string.a *.o

install_brew:
	cd ~
	curl -fsSL https://rawgit.com/kube/42homebrew/master/install.sh | zsh
	brew install lcov

clean_obj:
	@(rm -rf *.o)

clean_lib:
	@(rm -rf *.a)

clean_test:
	@(rm -rf *.gcda)
	@(rm -rf *.gcno)
	@(rm -rf *.info)
	@(rm -rf gcov_report)
	@(rm -rf report)

clean: clean_lib clean_lib clean_test clean_obj
	@(rm -rf test)
