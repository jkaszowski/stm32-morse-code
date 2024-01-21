SHELL   = /bin/sh

FILE0   = document
TEX     = $(FILE0).tex
XDV     = $(FILE0).xdv
PDF     = $(FILE0).pdf

all:
	make tex

tex:
	xelatex -no-pdf $(TEX)
	# xdvipdfmx.exe $(XDV)
	xelatex -synctex=1 -interaction=nonstopmode $(TEX)
	make clean

pw:
	zathura document.pdf &

help:
	echo "USAGE: make [all/tex/pw/clean]"

clean:
	rm -f *.aux *.dvi *.idx *.ilg *.ind *.log *.nav *.out *.snm *.xdv *.toc *~
