Route Backup
============

This program is designed to export and and import the settings of the `route` command found in most Linux distributions.

This could be useful for if the routing parameters need to persist when an ip address changes. Normally the `ifconfig`
command will by design flush the routing table.

## Usage

    Usage: rback [--export/--import] [-f export/import directory]

	    --export

	    	export the route table

	    --import

	    	import archived route information (this will not replace the data)

	    -f

	    	directory from where export/import will be performed