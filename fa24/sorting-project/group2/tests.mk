#########################################################
# TESTS += $(call test-name,tile_X,tile_Y,[buffer-size]) #
#########################################################
TESTS += $(call test-name,4,2,4096)
TESTS += $(call test-name,4,2,8192)
TESTS += $(call test-name,4,2,16384)

TESTS += $(call test-name,4,4,4096)
TESTS += $(call test-name,4,4,8192)
TESTS += $(call test-name,4,4,16384)

TESTS += $(call test-name,8,4,4096)
TESTS += $(call test-name,8,4,8192)
TESTS += $(call test-name,8,4,16384)

TESTS += $(call test-name,8,8,4096)
TESTS += $(call test-name,8,8,8192)
TESTS += $(call test-name,8,8,16384)

TESTS += $(call test-name,16,8,4096)
TESTS += $(call test-name,16,8,8192)
TESTS += $(call test-name,16,8,16384)
