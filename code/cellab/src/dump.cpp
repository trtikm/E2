#include <cellab/dump.hpp>
#include <ostream>

namespace cellab {


std::ostream&  dump_in_html(
        std::ostream&  ostr,
        std::shared_ptr<static_state_of_neural_tissue const> const  static_tissue_ptr
        )
{
    ostr << "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
            "    <title>Table of regions of distribution of degrees</title>\n"
            "    <style type=\"text/css\">\n"
            "        body {\n"
            "            background-color: white;\n"
            "            color: black;\n"
            "            margin-left: auto;\n"
            "            margin-right: auto;\n"
            "        }\n"
            "        h1, h2, h3, h4, h5, h6, table { font-family:\"Liberation serif\"; }\n"
            "        p, table {\n"
            "            font-size:12pt;\n"
            "            margin-left: auto;\n"
            "            margin-right: auto;\n"
            "            text-align: justify\n"
            "        }\n"
            "        th, td {\n"
            "            font-family:\"Liberation serif\";\n"
            "            font-size:12pt;\n"
            //"            padding: 3pt;\n"
            "        }\n"
            "   </style>\n"
            "</head>\n"
            "<body>\n"
            "<h2>Static state of neural tissue.</h2>\n"
            "<table>\n"
            "<tr>\n"
            "   <th align=\"right\">Property</th>\n"
            "   <th align=\"center\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Value&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>\n"
            "   <th align=\"left\">Description</th>\n"
            "</tr>\n"
            "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>K</i></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_kinds_of_tissue_cells() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A number of kinds of territories in the tissue.</td>\n"
            "</tr>\n"
            "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>S</i></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_kinds_of_sensory_cells() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A number of kinds of sensory cells.</td>\n"
            "</tr>\n"
            "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>U</i></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_kinds_of_synapses_to_muscles() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A number of kinds of synapses to muscles.</td>\n"
            "</tr>\n"
            ;
    ostr << "<tr><td></td><td></td><td></td></tr>   <tr><td></td><td></td><td></td></tr>\n";
    for (kind_of_cell  i = 0U; i < static_tissue_ptr->num_kinds_of_tissue_cells(); ++i)
    {
        ostr << "<tr>\n"
                "   <td align=\"right\" valign=\"top\"><i>N</i><sub>" << i << "</sub></td>\n"
                "   <td align=\"center\" valign=\"top\">"
                        << static_tissue_ptr->num_cells_of_cell_kind(i) <<
                        "</td>\n"
                "   <td align=\"left\" valign=\"top\">A count of tissue cells of the kind " << i << ".</td>\n"
                "</tr>\n"
                ;
    }
    ostr << "<tr><td></td><td></td><td></td></tr>   <tr><td></td><td></td><td></td></tr>\n";
    for (kind_of_cell  i = static_tissue_ptr->num_kinds_of_tissue_cells();
         i < static_tissue_ptr->num_kinds_of_cells(); ++i)
    {
        ostr << "<tr>\n"
                "   <td align=\"right\" valign=\"top\"><i>N</i><sub>" << i << "</sub></td>\n"
                "   <td align=\"center\" valign=\"top\">"
                        << static_tissue_ptr->num_cells_of_cell_kind(i) <<
                        "</td>\n"
                "   <td align=\"left\" valign=\"top\">A count of sensory cells of the kind " << i << ".</td>\n"
                "</tr>\n"
                ;
    }
    ostr << "<tr><td></td><td></td><td></td></tr>   <tr><td></td><td></td><td></td></tr>\n";
    for (kind_of_synapse_to_muscle  i = 0U; i < static_tissue_ptr->num_kinds_of_synapses_to_muscles(); ++i)
    {
        ostr << "<tr>\n"
                "   <td align=\"right\" valign=\"top\"><i>M</i><sub>" << i << "</sub></td>\n"
                "   <td align=\"center\" valign=\"top\">"
                        << static_tissue_ptr->num_synapses_to_muscles_of_kind(i) <<
                        "</td>\n"
                "   <td align=\"left\" valign=\"top\">A count of synapses to muscles of the kind " << i << ".</td>\n"
                "</tr>\n"
                ;
    }
    ostr << "<tr><td></td><td></td><td></td></tr>   <tr><td></td><td></td><td></td></tr>\n";
    for (kind_of_cell  i = 0U; i < static_tissue_ptr->num_kinds_of_tissue_cells(); ++i)
    {
        ostr << "<tr>\n"
                "   <td align=\"right\" valign=\"top\"><i>T</i><sub>" << i << "</sub></td>\n"
                "   <td align=\"center\" valign=\"top\">"
                        << static_tissue_ptr->num_synapses_in_territory_of_cell_kind(i) <<
                        "</td>\n"
                "   <td align=\"left\" valign=\"top\">A count of synapses in territory of kind " << i << ".</td>\n"
                "</tr>\n"
                ;
    }
    ostr << "<tr><td></td><td></td><td></td></tr>   <tr><td></td><td></td><td></td></tr>\n"
            "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>X</i></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_cells_along_x_axis() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A number of columns along X axis of the tissue.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>Y</i></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_cells_along_y_axis() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A number of columns along Y axis of the tissue.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>C</i></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_cells_along_columnar_axis() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A number of territories along columnar axis C of the tissue.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr><td></td><td></td><td></td></tr>   <tr><td></td><td></td><td></td></tr>\n"
            "<tr>\n"
            "   <td align=\"right\" valign=\"top\"></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << (natural_64_bit)static_tissue_ptr->num_cells_along_x_axis() *
                       (natural_64_bit)static_tissue_ptr->num_cells_along_y_axis() *
                       (natural_64_bit)static_tissue_ptr->num_cells_along_columnar_axis() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A total number of territories in the tissue.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr>\n"
            "   <td align=\"right\" valign=\"top\"></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << [](std::shared_ptr<static_state_of_neural_tissue const> const  static_tissue_ptr) {
                            natural_64_bit  num_synapses_in_column = 0ULL;
                            for (kind_of_cell  i = 0U; i < static_tissue_ptr->num_kinds_of_tissue_cells(); ++i)
                                num_synapses_in_column +=
                                    (natural_64_bit)static_tissue_ptr->num_tissue_cells_of_cell_kind(i) *
                                    (natural_64_bit)static_tissue_ptr->num_synapses_in_territory_of_cell_kind(i);
                            return num_synapses_in_column;
                        }(static_tissue_ptr) <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A total number of synapses in any column in the tissue.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr>\n"
            "   <td align=\"right\" valign=\"top\"></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << [](std::shared_ptr<static_state_of_neural_tissue const> const  static_tissue_ptr) {
                            natural_64_bit  num_synapses_in_column = 0ULL;
                            for (kind_of_cell  i = 0U; i < static_tissue_ptr->num_kinds_of_tissue_cells(); ++i)
                                num_synapses_in_column +=
                                    (natural_64_bit)static_tissue_ptr->num_tissue_cells_of_cell_kind(i) *
                                    (natural_64_bit)static_tissue_ptr->num_synapses_in_territory_of_cell_kind(i);
                            return (natural_64_bit)static_tissue_ptr->num_cells_along_x_axis() *
                                   (natural_64_bit)static_tissue_ptr->num_cells_along_y_axis() *
                                   num_synapses_in_column;
                        }(static_tissue_ptr) <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A total number of synapses in all territories in the tissue.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>N</i><sub>S</sub></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_sensory_cells() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A total number of sensory cells.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
    ostr << "<tr>\n"
            "   <td align=\"right\" valign=\"top\"><i>N</i><sub>M</sub></td>\n"
            "   <td align=\"center\" valign=\"top\">"
                    << static_tissue_ptr->num_synapses_to_muscles() <<
                    "</td>\n"
            "   <td align=\"left\" valign=\"top\">A total number of synapses to muscles.</td>\n"
            "</tr>\n"
            "<tr></tr>\n"
            ;
//    <tr><td align="right" valign="top">
//    <i>M</i><sub>X</sub>
//    </td><td></td><td align="left" valign="top">
//    A mode map (either clip or torus) for X axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>M</i><sub>Y</sub>
//    </td><td></td><td align="left" valign="top">
//    A mode map (either clip or torus) for Y axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>M</i><sub>C</sub>
//    </td><td></td><td align="left" valign="top">
//    A mode map (either clip or torus) for the columnar axis C of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RC</i><sub>X</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for tissue cells in X axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RC</i><sub>Y</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for tissue cells in Y axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RC</i><sub>C</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for tissue cells in the columnar C axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RS</i><sub>X</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for signallings in X axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RS</i><sub>Y</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for signallings in Y axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RS</i><sub>C</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for signallings in the columnar C axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RT</i><sub>X</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for triples over territorial lists <i>LS</i>, <i>LT</i>,
//    and <i>LC</i> (see the lists
//    <a href="./cellab.html#territorial_lists">here</a>)
//    in X axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RT</i><sub>Y</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for triples over territorial lists <i>LS</i>, <i>LT</i>,
//    and <i>LC</i> (see the lists
//    <a href="./cellab.html#territorial_lists">here</a>)
//    in Y axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>RT</i><sub>C</sub>
//    </td><td></td><td align="left" valign="top">
//    Arrays of <i>K</i> radii for triples over territorial lists <i>LS</i>, <i>LT</i>,
//    and <i>LC</i> (see the lists
//    <a href="./cellab.html#territorial_lists">here</a>)
//    in the columnar C axis of the tissue.
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>NB</i><sub>C</sub>
//    </td><td></td><td align="left" valign="top">
//    A number of bits reserved for a tissue cell. The number must be aligned to 8 bits
//    (i.e. a multiple of 8).
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>NB</i><sub>S</sub>
//    </td><td></td><td align="left" valign="top">
//    A number of bits reserved for a synapse. The number must be aligned to 8 bits
//    (i.e. a multiple of 8).
//    </td></tr>
//    <tr><td align="right" valign="top">
//    <i>NB</i><sub>G</sub>
//    </td><td></td><td align="left" valign="top">
//    A number of bits reserved for a signalling. The number must be aligned to 8 bits
//    (i.e. a multiple of 8).
//    </td></tr>



    ostr << "</table>\n"
            "</body>\n"
            "</html>\n"
            ;
    return ostr;
}


}
