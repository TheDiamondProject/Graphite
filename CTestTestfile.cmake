set(TEST_TARGET_PATH ${CMAKE_SOURCE_DIR}/bin/Graphite_TestRunner -s)

add_test(NAME ConstructDataBlock_Power2Capacity
        COMMAND ${TEST_TARGET_PATH} ConstructDataBlock_Power2Capacity)

add_test(NAME ConstructDataBlock_NonePower2Capacity
        COMMAND ${TEST_TARGET_PATH} ConstructDataBlock_NonePower2Capacity)