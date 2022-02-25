commands 1
    set $GAMA_SO_LIB="/home/alex/Desktop/gamademo/auto/injector_test_demo/injector.so"
    # set $IN_PROGRESS="/home/alex/Desktop/gammaray/build-gammaray-2.9.0-unknown-Debug/lib/gammaray/2.9/qt5_11-x86_64/gammaray_inprocessui.so"
    # set logging on
    set confirm off
    #set unwindonsignal on
    sha dl
    call (void) dlopen($GAMA_SO_LIB, 2)
    sha $GAMA_SO_LIB
    
    #call (void) dlopen($IN_PROGRESS, 2)
    #sha $IN_PROGRESS
    call (void) gammaray_probe_attach()
    #call (void) gammaray_create_inprocess_mainwindow()
end

# detach
# quit

#b propertydata.cpp:56
#commands 2
#    p c_value
#    continue
#end
