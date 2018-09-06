#ifndef OPENMC_CELL_H
#define OPENMC_CELL_H

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "hdf5.h"
#include "pugixml.hpp"

#include "openmc/constants.h"
#include "openmc/position.h"

#ifdef CAD
#include "DagMC.hpp"
#endif

namespace openmc {

//==============================================================================
// Constants
//==============================================================================

// TODO: Convert to enum
extern "C" int FILL_MATERIAL;
extern "C" int FILL_UNIVERSE;
extern "C" int FILL_LATTICE;

// TODO: Convert to enum
constexpr int32_t OP_LEFT_PAREN   {std::numeric_limits<int32_t>::max()};
constexpr int32_t OP_RIGHT_PAREN  {std::numeric_limits<int32_t>::max() - 1};
constexpr int32_t OP_COMPLEMENT   {std::numeric_limits<int32_t>::max() - 2};
constexpr int32_t OP_INTERSECTION {std::numeric_limits<int32_t>::max() - 3};
constexpr int32_t OP_UNION        {std::numeric_limits<int32_t>::max() - 4};

//==============================================================================
// Global variables
//==============================================================================

extern "C" int32_t n_cells;

class Cell;
extern std::vector<Cell*> cells;
extern std::unordered_map<int32_t, int32_t> cell_map;

class Universe;
extern std::vector<Universe*> universes;
extern std::unordered_map<int32_t, int32_t> universe_map;

//==============================================================================
//! A geometry primitive that fills all space and contains cells.
//==============================================================================

class Universe
{
public:
  int32_t id_;                  //!< Unique ID
  std::vector<int32_t> cells_;  //!< Cells within this universe

  //! \brief Write universe information to an HDF5 group.
  //! \param group_id An HDF5 group id.
  void to_hdf5(hid_t group_id) const;
};

//==============================================================================
//! A geometry primitive that links surfaces, universes, and materials
//==============================================================================

class Cell
{
public:
  int32_t id_;                //!< Unique ID
  std::string name_;          //!< User-defined name
  int type_;                  //!< Material, universe, or lattice
  int32_t universe_;          //!< Universe # this cell is in
  int32_t fill_;              //!< Universe # filling this cell
  int32_t n_instances_{0};    //!< Number of instances of this cell

  //! \brief Index corresponding to this cell in distribcell arrays
  int distribcell_index_{C_NONE};

  //! \brief Material(s) within this cell.
  //!
  //! May be multiple materials for distribcell.
  std::vector<int32_t> material_;

  //! \brief Temperature(s) within this cell.
  //!
  //! The stored values are actually sqrt(k_Boltzmann * T) for each temperature
  //! T. The units are sqrt(eV).
  std::vector<double> sqrtkT_;

  //! Definition of spatial region as Boolean expression of half-spaces
  std::vector<std::int32_t> region_;
  //! Reverse Polish notation for region expression
  std::vector<std::int32_t> rpn_;
  bool simple_;  //!< Does the region contain only intersections?

  Position translation_ {0, 0, 0}; //!< Translation vector for filled universe

  //! \brief Rotational tranfsormation of the filled universe.
  //
  //! The vector is empty if there is no rotation.  Otherwise, the first three
  //! values are the rotation angles respectively about the x-, y-, and z-, axes
  //! in degrees.  The next 9 values give the rotation matrix in row-major
  //! order.
  std::vector<double> rotation_;

  std::vector<int32_t> offset_;  //!< Distribcell offset table

  explicit Cell(pugi::xml_node cell_node);
  Cell() {};


  //! \brief Determine if a cell contains the particle at a given location.
  //!
  //! The bounds of the cell are detemined by a logical expression involving
  //! surface half-spaces. At initialization, the expression was converted
  //! to RPN notation.
  //!
  //! The function is split into two cases, one for simple cells (those
  //! involving only the intersection of half-spaces) and one for complex cells.
  //! Simple cells can be evaluated with short circuit evaluation, i.e., as soon
  //! as we know that one half-space is not satisfied, we can exit. This
  //! provides a performance benefit for the common case. In
  //! contains_complex, we evaluate the RPN expression using a stack, similar to
  //! how a RPN calculator would work.
  //! \param r The 3D Cartesian coordinate to check.
  //! \param u A direction used to "break ties" the coordinates are very
  //!   close to a surface.
  //! \param on_surface The signed index of a surface that the coordinate is
  //!   known to be on.  This index takes precedence over surface sense
  //!   calculations.
  virtual bool
  contains(Position p, Direction u, int32_t on_surface) const = 0;

  //! Find the oncoming boundary of this cell.
  virtual std::pair<double, int32_t>
  distance(Position p, Direction u, int32_t on_surface) const = 0;

  //! Write all information needed to reconstruct the cell to an HDF5 group.
  //! @param group_id An HDF5 group id.
  virtual void to_hdf5(hid_t group_id) const = 0;

};
 
class CSGCell : public Cell
{
public:

  CSGCell();
  
  explicit CSGCell(pugi::xml_node cell_node);

  bool
  contains(Position r, Direction u, int32_t on_surface) const;

  std::pair<double, int32_t>
  distance(Position r, Direction u, int32_t on_surface) const;

  void to_hdf5(hid_t group_id) const;

protected:
  bool contains_simple(Position r, Direction u, int32_t on_surface) const;
  bool contains_complex(Position r, Direction u, int32_t on_surface) const;
};

#ifdef CAD
class CADCell : public Cell
{
 public:
  moab::DagMC *dagmc_ptr;
  explicit CADCell();

  std::pair<double, int32_t> distance(Position p, Direction u, int32_t on_surface) const;
  bool contains(Position p, Direction u, int32_t on_surface) const;

  void to_hdf5(hid_t group_id) const;

};
#endif
 
} // namespace openmc
#endif // OPENMC_CELL_H
