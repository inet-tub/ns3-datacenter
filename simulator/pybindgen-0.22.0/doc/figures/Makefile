
figures_dia=$(shell echo *.dia)
figures_png=$(figures_dia:.dia=.png)


all: $(figures_png)

%.png: %.dia
	dia $< -e $@


clean:
	-rm -f $(figures_png)
