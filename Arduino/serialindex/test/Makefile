all:
	./generate_sample.sh
	g++ -Wall -o test test.cpp ../IO.cpp ../util.cpp -I ..

clean:
	-@rm serialindex_test 2> /dev/null || true
