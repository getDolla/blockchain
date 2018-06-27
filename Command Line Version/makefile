G++ = g++ -std=c++11
input = main
output = $(input).out

compile:
	$(G++) -o $(output) $(input).cpp sha256.cpp base64.cpp

# Compile and run
run: compile
	./$(output)

gdb: compile
		gdb $(output)

clean:
	rm -f $(output)
	rm -f *.out
	rm -f *~
