test_suite(Graphite

    # Data
        # Data Block
            dataBlock_constructWithPowerOfTwoCapacity_sizeIsCorrect
            dataBlock_constructWithCapacity_rawSizeIsAdjustedCorrectly
            dataBlock_construct_defaultByteOrderIsCorrect
            dataBlock_construct_usingLSBByteOrder_assignsCorrectly
            dataBlock_construct_hasOwnershipOfInitialData
            dataBlock_getByteValue_atOffset_isExpectedValueReturned
            dataBlock_getShortValue_atOffset_isExpectedValueReturned
            dataBlock_getLongValue_atOffset_isExpectedValueReturned
            dataBlock_getQuadValue_atOffset_isExpectedValueReturned

    # QuickDraw
        # Point
            point_constructUsingEqualCoordinates
            point_constructUsingSeparateCoordinates
            point_constructUsingReader_quickdraw
            point_constructUsingReader_macintosh
            point_readPointFromReader
            point_encodeInToWriter_quickdraw
            point_encodeInToWriter_macintosh
            point_assignmentOperator_copyPoint
            point_assignmentOperator_movePoint
            point_equalsOperator_shouldBeEqual
            point_equalsOperator_shouldNotBeEqual
            point_notEqualsOperator_shouldNotBeEqual
            point_notEqualsOperator_shouldBeEqual
            point_addPoint_returnsExpectedPoint
            point_subtractPoint_returnsExpectedPoint
            point_multiplyPoint_returnsExpectedPoint
            point_dividePoint_returnsExpectedPoint
            point_castToDifferentType_retainsSameValue

        # Size
        # Rect

    # Utility Functions
        # XXHash Hashing
            xxh64_verifyCorrectHashIsProduced

)
