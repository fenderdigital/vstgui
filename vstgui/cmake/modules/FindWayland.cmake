# This file is part of VSTGUI. It is subject to the license terms
# in the LICENSE file found in the top-level directory of this
# distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
# Originally written and contributed to VSTGUI by PreSonus Software Ltd.

include_guard (GLOBAL)

find_package (PkgConfig)
pkg_check_modules (PKG_WAYLAND QUIET wayland-client)

set (WAYLAND_DEFINITIONS ${PKG_WAYLAND_CFLAGS})

# Find wayland libraries / includes
# When cross-compiling, these libraries and headers need to be found for the target architecture.
find_path (WAYLAND_CLIENT_INCLUDE_DIR NAMES wayland-client.h HINTS ${PKG_WAYLAND_INCLUDE_DIRS} ONLY_CMAKE_FIND_ROOT_PATH)
mark_as_advanced (WAYLAND_CLIENT_INCLUDE_DIR)

if ("client" IN_LIST Wayland_FIND_COMPONENTS)
	find_library (WAYLAND_CLIENT_LIBRARIES NAMES wayland-client HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
	find_library (WAYLAND_CURSOR_LIBRARIES NAMES wayland-cursor HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
endif ()

if ("egl" IN_LIST Wayland_FIND_COMPONENTS)
	find_library (WAYLAND_EGL_LIBRARIES NAMES wayland-egl HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
endif ()

if ("server" IN_LIST Wayland_FIND_COMPONENTS)
	find_library (WAYLAND_SERVER_LIBRARIES NAMES wayland-server HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
endif ()

if ("protocols" IN_LIST Wayland_FIND_COMPONENTS)

	# Find wayland-scanner
	find_program (WAYLAND_SCANNER NAMES "wayland-scanner")
	if (NOT WAYLAND_SCANNER)
		find_program (WAYLAND_SCANNER NAMES wayland-scanner)
	endif ()
	mark_as_advanced (WAYLAND_SCANNER)

	# Generate extra protocol headers

	find_path (XDG_DIR NAMES "stable/xdg-shell/xdg-shell.xml" HINTS "/usr/share/wayland-protocols")
	mark_as_advanced (XDG_DIR)

	set (protocols
		"stable/xdg-shell/xdg-shell.xml"
		#"staging/xdg-activation/xdg-activation-v1.xml"
		#"unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
		#"unstable/xdg-foreign/xdg-foreign-unstable-v1.xml"
		#"unstable/xdg-foreign/xdg-foreign-unstable-v2.xml"
		#"unstable/text-input/text-input-unstable-v3.xml"
	)

	set (WAYLAND_PROTOCOLS_DIR "${CMAKE_BINARY_DIR}/wayland-protocols" CACHE PATH "Directory for generated wayland protocol headers and source files")
	file (MAKE_DIRECTORY ${WAYLAND_PROTOCOLS_DIR})

	foreach (protocol ${protocols})

		get_filename_component (protocol_name "${protocol}" NAME_WLE)

		# Generate client header
		set (header "${WAYLAND_PROTOCOLS_DIR}/${protocol_name}-client-protocol.h")
		if (NOT EXISTS "${XDG_DIR}/${protocol}")
			message (WARNING "Unknown Wayland protocol: ${protocol_name}")
			file (WRITE "${header}" "")
			continue ()
		endif ()

		add_custom_command (OUTPUT ${header}
			COMMAND /bin/sh -c "${WAYLAND_SCANNER} client-header < ${XDG_DIR}/${protocol} > \"${header}\""
			DEPENDS ${XDG_DIR}/${protocol}
			VERBATIM USES_TERMINAL
		)
		list (APPEND protocol_headers ${header})

		# Generate server header
		set (header "${WAYLAND_PROTOCOLS_DIR}/${protocol_name}-server-protocol.h")

		add_custom_command (OUTPUT ${header}
			COMMAND /bin/sh -c "${WAYLAND_SCANNER} server-header < ${XDG_DIR}/${protocol} > \"${header}\""
			DEPENDS ${XDG_DIR}/${protocol}
			VERBATIM USES_TERMINAL
		)
		list (APPEND protocol_headers ${header})

		# Generate source file
		set (sourcefile "${WAYLAND_PROTOCOLS_DIR}/${protocol_name}-protocol.c")
		add_custom_command (OUTPUT ${sourcefile}
			COMMAND /bin/sh -c "${WAYLAND_SCANNER} private-code < ${XDG_DIR}/${protocol} > \"${sourcefile}\""
			DEPENDS ${XDG_DIR}/${protocol}
			VERBATIM USES_TERMINAL
		)
		list (APPEND protocol_source_files "${sourcefile}")
	endforeach ()

	add_library (wayland_protocols OBJECT ${protocol_headers} ${protocol_source_files} ${CMAKE_CURRENT_LIST_FILE})

	set_target_properties (wayland_protocols PROPERTIES
		USE_FOLDERS ON
		FOLDER libs
	)

endif ()

# Set result variables
set (WAYLAND_LIBRARIES ${WAYLAND_CLIENT_LIBRARIES} ${WAYLAND_CURSOR_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${WAYLAND_SERVER_LIBRARIES})

set (WAYLAND_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIR} ${WAYLAND_PROTOCOLS_DIR})
list (REMOVE_DUPLICATES WAYLAND_INCLUDE_DIRS)

if (TARGET wayland_protocols)
	list (APPEND WAYLAND_LIBRARIES wayland_protocols)
	target_include_directories (wayland_protocols PUBLIC ${WAYLAND_INCLUDE_DIRS})
endif ()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Wayland
	FOUND_VAR WAYLAND_FOUND
	REQUIRED_VARS WAYLAND_LIBRARIES WAYLAND_INCLUDE_DIRS
)
