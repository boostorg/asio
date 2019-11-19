PNG=async_op1.png async_op2.png proactor.png sync_op.png

all:	$(PNG)

clean:
	$(RM) *~

distclean:	clean
	$(RM) $(PNG)

%.png:	%.dot
	dot -Gdpi=150 -Tpng $< > $@
