// Copyright (C) 2008  Andreas Stewering
//
// This file is part of Etherlab.
//
// Etherlab is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Etherlab is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Etherlab; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA//
// ====================================================================

//Check for existing Description
function slaveid = getslavedesc_checkslave(slave_desc,slavetype,rev)
  numtypes = getslavedesc_numtypes(slave_desc);
  slaveid=-1;
  for i=1:numtypes
    slavename = getslavedesc_typename(slave_desc,i);
    if slavename == slavetype
       revision = getslavedesc_revision(slave_desc,i);
       if revision == rev
         slaveid = i;
       end
    end
  end
endfunction