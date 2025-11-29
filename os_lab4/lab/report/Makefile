.PHONY: all clean

# Имя основного файла
MAIN = report

# Компилятор LaTeX
LATEX = pdflatex

all: $(MAIN).pdf

$(MAIN).pdf: $(MAIN).tex
	$(LATEX) $(MAIN).tex
	$(LATEX) $(MAIN).tex

clean:
	rm -f $(MAIN).aux $(MAIN).log $(MAIN).out $(MAIN).toc $(MAIN).pdf



