# PyCVM Examples

These examples show how to do various simple things with the Python APIs
that come with UCVM 14.3.0 and above. Documentation on how to run the 
examples are provided within the code itself.

A brief description of what each script does is as follows:

- depth_profile_ucvm15100.py: Creates a simple depth profile from
  CVM-H and saves it to a file.
- depth_profiles_cvms426_ucvm15100.py: Creates a suite of profiles from
  a given velocity model, points, etc. and saves them as well as a text
  description of the material properties to various files.
- extract_horizontal_slice_text.py: Creates a suite of text-based 
  horizontal slices from a given set of one or more velocity models at
  one or more specified depths. This allows for specifying relatively
  arbitrary parallelograms.