CHANGELOG
=========

The format is based on `Keep a
Changelog <https://keepachangelog.com/en/1.0.0/>`__, and this project
adheres to `Semantic
Versioning <https://semver.org/spec/v2.0.0.html>`__.

.. _310--Unreleased:

[3.1.0] - 2022-07-23
--------------------

Added
~~~~~

- Add check for ``LLMJVM_IMPL_setApplicationTime``.
- Test Thread.sleep() with max number of milliseconds (Long.MAX_VALUE) does not cause an infinite loop in the MicroEJ Core due to time conversion overflow.
- Add check for ``LLMJVM_IMPL_getCurrentTime``, which should not go back in time.
- Add check for C malloc heap configuration which can be used when parsing float.
- Add check of ``LLMJVM_IMPL_scheduleRequest``, ``LLMJVM_IMPL_getCurrentTime``, and ``LLMJVM_IMPL_getTimeNanos`` precision

Changed
~~~~~~~

- Split floating-point tests into 3 JUnit tests (FPU, parser, formatter)
  to allow for fine-grained ignored tests list.
- Reformat to use standard MicroEJ Test Suite flow.
- Add tolerance on floating-point parsing.

Fixed
~~~~~

- Fix assertions to conform with JUnit semantic of expected/actual.

.. _302--2021-04-14:

[3.0.2] - 2021-04-14
--------------------

Fixed
~~~~~

- Ensure an error message is shown when the Round Robin test fails.

.. _301--2021-02-12:

[3.0.1] - 2021-02-12
--------------------

Fixed
~~~~~

- Removed a comment about ``testFloat`` and ``testDouble`` functions
  in ``MicroEJCoreValidation.java``.  This is documented in the README
  of the project.

.. _300---2020-06-02:

[3.0.0] - 2020-06-02
--------------------

Changed
~~~~~~~

-  Previous test functions are now JUnit test cases

.. _240---2019-12-18:

[2.4.0] - 2019-12-18
--------------------

Added
~~~~~

-  New floating-point related tests: parse/toString for float/double

.. _changed-1:

Changed
~~~~~~~

-  Review round robin check test
-  Update license

.. _230---2019-10-01:

[2.3.0] - 2019-10-01
--------------------

Initial revision with Changelog.

.. _added-1:

Added
~~~~~

-  Property ``MJVMPortValidation.clock.seconds`` to configure the time
   in seconds of the visible clock test (default to 10s).

.. _100---2017-09-29:

[1.0.0] - 2017-09-29
--------------------

Features : - Initial revision

..
    Copyright 2020-2022 MicroEJ Corp. All rights reserved.
    Use of this source code is governed by a BSD-style license that can be found with this software.
