all: install regex_demo

install: clean build
	python setup.py develop --user

build:
	python setup.py build --force

clean:
	python setup.py develop --user -u
	python setup.py clean --all
	rm -rf build install dist jellyfish.egg-info regex_demo
	find . -name "*.pyc" -delete

regex_demo:
	gcc -Wall -o regex_demo -std=c11 -g -pg regex_demo.c ./jellyfish.so

test:
	nosetests -v -v test.py
