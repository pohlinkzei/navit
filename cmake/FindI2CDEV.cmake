find_path (I2CDEV_INCLUDE_DIR "linux/i2c-dev.h")


set (I2CDEV_INCLUDE_DIRS ${I2CDEV_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set I2CDEV_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(I2CDEV DEFAULT_MSG I2CDEV_INCLUDE_DIR)

mark_as_advanced (I2CDEV_INCLUDE_DIR )
