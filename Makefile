
.PHONY: docs clean
.DEFAULT: docs

docs: Makefile
	doxygen doxygen.cfg


#remove all intermediate files
clean:
	-@$(RM) obj/*.o obj/*.d include/*.hh include/yyparser.h src/parser.cpp
