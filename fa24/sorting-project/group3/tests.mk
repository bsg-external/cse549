#########################################################
# TESTS += $(call test-name,[buffer-size]) #
#########################################################
TESTS += $(call test-name,8)
TESTS += $(call test-name,125)
TESTS += $(call test-name,128)
TESTS += $(call test-name,133)
TESTS += $(call test-name,256)
TESTS += $(call test-name,512)
TESTS += $(call test-name,1024)
# TESTS += $(call test-name,2048)
# TESTS += $(call test-name,4096)
# TESTS += $(call test-name,16384)
# TESTS += $(call test-name,65536)
# TESTS += $(call test-name,262144)
# TESTS += $(call test-name,1048576)
# TESTS += $(call test-name,4194304)
# TESTS += $(call test-name,16777216)