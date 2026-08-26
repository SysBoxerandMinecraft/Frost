# tools/compat_registry/Makefile

obj-y := lib/
obj-y += cmd/
obj-y += explorer/
obj-y += sas/
obj-y += uac/
obj-y += runbox/

all: $(obj-y)

lib:
	$(Q)$(MAKE) -C $@

cmd: lib
	$(Q)$(MAKE) -C $@

explorer: lib
	$(Q)$(MAKE) -C $@

sas: lib
	$(Q)$(MAKE) -C $@

uac: lib
	$(Q)$(MAKE) -C $@

runbox: lib
	$(Q)$(MAKE) -C $@

clean:
	$(Q)$(MAKE) -C lib clean
	$(Q)$(MAKE) -C cmd clean
	$(Q)$(MAKE) -C explorer clean
	$(Q)$(MAKE) -C sas clean
	$(Q)$(MAKE) -C uac clean
	$(Q)$(MAKE) -C runbox clean

.PHONY: all lib cmd explorer sas uac runbox clean