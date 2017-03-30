Release Verification Tests
==========================

Before UCVM is released to the public, we run an automated suite of tests to ensure that the platform works as expected.
The following tests are run before each release and must pass. The command that is run is given first, followed by the
input that is provided, and then the expected output. We also list if a model is required in order to do this test
yourself.

These tests are all automated. The tests themselves reside in a SQLite database within the distribution
(ucvm/tests/data/commands.db) and the command to re-run all of these tests is python3 ucvm/tests/commands.py from
the Git source distribution (after you have installed and sourced in UCVM of course).

Only one "output" is provided. This is both the expected output (i.e. what we are testing against during release) and
what the actual test output is, since all of these tests must pass before UCVM is released to the public.

.. include:: tests_rv_include.rst