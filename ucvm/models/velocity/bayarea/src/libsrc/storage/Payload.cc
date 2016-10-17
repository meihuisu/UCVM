// -*- C++ -*-
//
// ======================================================================
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ======================================================================
//

#include "Payload.h" // implementation of class methods

// ----------------------------------------------------------------------
const char* cencalvm::storage::Payload::SCHEMA = 
  "float32_t Vp; "
  "float32_t Vs; "
  "float32_t Density; "
  "float32_t Qp; "
  "float32_t Qs; "
  "float32_t DepthFreeSurf; "
  "int16_t FaultBlock; "
  "int16_t Zone;";

const float cencalvm::storage::Payload::NODATAVAL = -999.0;
const short cencalvm::storage::Payload::NODATABLOCK = 0;
const short cencalvm::storage::Payload::NODATAZONE = 0;

const short cencalvm::storage::Payload::INTERIORBLOCK = -999;
const short cencalvm::storage::Payload::INTERIORZONE = -999;

// version
// $Id: Payload.cc 2010 2005-10-11 18:27:42Z brad $

// End of file 
