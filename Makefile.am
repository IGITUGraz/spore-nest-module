# Automake file for external dynamic modules for NEST
#
# Hans Ekkehard Plesser, April 2008
# Automake file for the Developer Module
#
# libsporemodule is built as a normal, installable library.
# It will be installed to $prefix/lib by make install.
#
# Headers from this directory are not to be installed upon
# make install. They are therefore included in _SOURCES.


# 1. Exchange "my" in "sporemodule" with the name of your model below
#    (ten locations).
# 2. Add all .cpp and .h files from your code as *_SOURCES. Header files
#    are given only so that they will be included in the tarball if you
#    run "make dist" on your module.
# 3. The sporemodule* stuff creates a module that can be loaded at runtime.
#    It is called sporemodule.so.
# 4. The libsporemodule* stuff creates a library against which NEST can be
#    linked.

libdir= @libdir@/nest

lib_LTLIBRARIES=      sporemodule.la libsporemodule.la

sporemodule_la_CXXFLAGS= -g @AM_CXXFLAGS@ -std=c++11
sporemodule_la_SOURCES=  sporemodule.cpp sporemodule.h \
                         connection_updater.cpp connection_updater.h \
                         connection_data_logger.cpp connection_data_logger.h \
                         tracing_node.cpp tracing_node.h \
                         poisson_dbl_exp_neuron.cpp poisson_dbl_exp_neuron.h \
                         diligent_connector_model.h \
                         synaptic_sampling_rewardgradient_connection.h \
                         reward_in_proxy.h reward_in_proxy.cpp \
                         spore_test_node.h spore_test_node.cpp \
                         spore_test_connection.h \
                         spore_test_base.h \
                         test_circular_buffer.h test_circular_buffer.cpp \
                         test_tracing_node.h test_tracing_node.cpp

sporemodule_la_LDFLAGS= -g -module

libsporemodule_la_CXXFLAGS= $(sporemodule_la_CXXFLAGS) -DLINKED_MODULE -std=c++11
libsporemodule_la_SOURCES=  $(sporemodule_la_SOURCES)

MAKEFLAGS= @MAKE_FLAGS@

AM_CPPFLAGS= @NEST_CPPFLAGS@ \
             @INCLTDL@

.PHONY: install-slidoc

pkgdatadir=@datadir@/nest

nobase_pkgdata_DATA=\
	sli/sporemodule-init.sli

install-slidoc:
	NESTRCFILENAME=/dev/null $(DESTDIR)$(NEST_PREFIX)/bin/sli --userargs="@HELPDIRS@" $(NEST_PREFIX)/share/nest/sli/install-help.sli

install-data-hook: install-exec install-slidoc

EXTRA_DIST= sli
