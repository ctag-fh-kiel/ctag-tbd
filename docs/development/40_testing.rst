*******
Testing
*******

Due to the modular and configurable nature of TBD firmwares, testing your code can have an additional dimension,
beyond the usual layers of testing (unit tests, integration tests, ...): Since each component can be built on different
platforms with different configurations, tests need to be run for every individual firmware.

Therefore each component should add it's own set of tests that can be run for a specific TBD configuration.


TBD Unit Tests
==============

TBD allows components to provide their own set of Google Test unittests, that can be run using the PlatformIO google
test runner, for every generated build. Unittests are intended to be runnable on every platform (including desktop
computers) and should therefore not contain any microcontroller specific code.

.. note::

    Unit tests have their own PlatformIO environment :code:`test`. Therefore you need to select activeate the
    :code:`test` environment in your IDE or use the :code:`-e test` flag on the PlatformIO CLI to run unittests.

Each component can provide its own tests by placing source files prefixed with ``test_`` in a ``test`` subdirectory
in the component's root directory. These tests, can utilize any component that the component directly depends on, they
should however not rely on any components that are not dependencies of the component.

To create your own unittest simple create a

.. code-block::

    some_component/test

directory in the components root directory. And add a unittest source file

.. code-block::

    some_component/test/test_something.cpp

Your test will need to include the Google Test headers, so you can use the :code:`TEST` macro to define tests:

.. code-block:: c++

    #include <gtest/gtest.h>

    TEST(TestSomething, it_can_do_foo) {
        // test something doing foo
    }

    TEST(TestSomething, it_can_do_bar) {
        // test something doing bar
    }

Include the specific interfaces of your component and use the Google Test assertion macro:

.. code-block:: c++

    #include <gtest/gtest.h>

    #include <tbd/some_component/foo.hpp>
    #include <tbd/errors.hpp>

    TEST(TestSomething, it_can_do_foo) {
        int a = 1;
        const auto res = tbd::some_component::foo(a);

        EXPECT_EQ(res, TBD_OK);
        EXPECT_EQ(a, 42);
    }

    // more tests

Finally you can run the entire test suite of a TBD firmware, by navigating to the firmware build directory and running

.. code-block:: bash

     pio test -e tests


TBD Build Generator Tests
=========================

.. note::

    Build generator tests are only relevant to core contributors.

For testing the python code of the TBD build generator or other parts of the TBD repo that are universal in the sense
that they are not specific to any firmware configuration can utilize the Python `unittest` framework and can be placed
in the :code:`tests` directory of the TBD repo itself.