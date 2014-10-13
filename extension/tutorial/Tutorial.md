This tutorial gives instructions on how to use the JS API for CrossWalk. The
tutorial is divided among a couple of high-level use-cases, such as browsing,
searching and playing. This is a work in progress, and is not yet complete.

## General workflow using the API
The intended usage of the API is to first register a notification handler using the {@link registerNotificationHandler} function. Using this function, the CrossWalk application can get notifications of events such as advancing to the next track, or when playback status or play order changes. This can be useful to trigger HMI transitions.

Media is generally intended to be browsed using the Media Manager. Due to the layout of the virtual directories (Artists, Albums, Genres on top level), searching the root directory is not very practical. Searching the root will (correctly) yield duplicate results from these virtual directories. The recommended use is thus instead to browse the desired virtual directory, and if necessary, search one of these virtual directories. Searching is covered in the Searching topic of this document.

## Browsing

For browsing, six functions are provided; {@link Browser#listContainers}, {@link Browser#listContainersEx}, {@link Browser#listItems}, {@link Browser#listItemsEx}, {@link Browser#listChildren}, and {@link Browser#listChildrenEx}. The functions suffixed with Ex provide sorting functionality (see the respective functions documentation, and the Sorting chapter of this document). The {@link Player#listContainers} functions are meant to be used to display only the containers(/virtual directories) in a container. This can be used to build "directory trees" where each item is browsable. The listItems functions are used to display the playable items within containers. The {@link Player#listItems} functions display both items and containers.

If we imagine the browsing functions as tools to traverse a tree, the {@link Player#listContainers} functions will traverse and display nodes, while the listItems will only display leaves, and {@link Player#listChildren} will display both.

For simplicity, it is recommended to use listItems and {@link Player#listContainers}, but this is really up to the HMI implementation.


## Playing
When media has been found, there are three ways to play it; {@link Player#enqueueUri}, {@link Player#openPlaylist} and {@link Player#openUri}. The {@link Player#enqueueUri} function will enqueue one track at the end of the current play queue. The {@link Player#openPlaylist} function will replace the current play queue with the contents of a container, and the {@link Player#openUri} function will, without enqueueing, start playing a specified track.

The recommended usage is to use {@link Player#enqueueUri} and {@link Player#openPlaylist}, while {@link Player#openUri} can be used for testing purposes.

Once a play queue has been constructed, the {@link Player#dequeueIndex}, and {@link Player#emptyPlayQueue} functions can be used to manipulate the play queue.

It is possible to query the playback state using various getter functions, see {@link Player} for a comprehensive list. Playing tracks from the play queue, pausing, skipping to previous and next tracks is done using the {@link Player#play}, {@link Player#pause}, {@link Player#previous}, {@link Player#next} respectively. The reader is encouraged to look at all the available functions and read their respective documentation.

## Searching
Searching is done using the {@link Browser#searchObjects} and {@link Browser#searchObjectsEx} functions. The difference between the two functions is that the Ex version allows sorting the results. See {@link Sorting} for an explanation on how sorting works in general. Both {@link Browser#searchObjects} and {@link Browser#searchObjectsEx} share a common query language. The query language is defined in the {@link https://wiki.gnome.org/Projects/Rygel/MediaServer2Spec | MediaServer2 specification}. The fields available in the queries are however implementation dependent, and are thus not a part of the MediaServer2 spec.

The fields available for queries also vary within the Media Manager, depending on the type of query being issued. The searching procedure can be seen as a way to narrow down a selection from the set of all objects (items and containers), and to narrow them down, we need to select among the fields of the result object. This means we can only use the field names of our expected result object in our queries. This sounds more complicated than it is, here are a few examples of queries:

    (1) TypeEx derivedFrom "container.music.musicAlbum" and Artist contains "Mado"

    (2) TypeEx derivedFrom "music" and Artist doesNotContain "Nickelback"

    (3) TypeEx derivedFrom "music" and DisplayName contains "Där dit vinden kommer" and Artist contains "Lorentz"

The first query will select all albums/media containers where the Artist field contains the string "Mado" - which means, for instance, if we have any albums by Madonna, there will be selected. Note that this will *only* return the actual albums with Madonna performing, and not the tracks in the albums.

The second query will return all tracks where the artist is not "Nickelback"

The third query will return the track named "Dit där vinden kommer" with the artist named "Lorentz".

The TypeEx, Artist and DisplayName fields are examples of available field names. The Artist field name will only be available for music items and containers, and not for instance when navigating the (not yet) indexed file system. In order to determine which field names you have available, please use the {@link Browser#listItems} or {@link Browser#listContainers} functions to inspect the respective objects.

The {@link Browser#searchObjects} and {@link Browser#searchObjectsEx} functions, just like the browsing functions, allow filtering of result fields. Please see the {@link Filtering} section for information on filtering. Note that the fields used in your search query do not have to be included in the filter-set of fields. This means you can search by Artist, even if the Artist field is not in your filter.

## Sorting
Sorting is inspired by the dLeyna API (which is used in the Media Manager for communicating with Rygel). The relevant documentation for the dLeyna API can be found {@link https://01.org/dleyna/documentation/server-objects | here}. In the Media Manager, all sorting is done using Ex functions, for instance {@link Browser#searchObjectsEx}. The Ex suffix comes from dLeyna naming, and doesn't really have any meaning in the Media Manager, but is kept for isomorphism between the APIs.

Each Ex function contain a sortKey parameter. This parameter allows specifying the sort order of the results returned from the respective function. Here are some examples of valid sort keys:

    +Artist
    -Artist
    +DisplayName

The + prefix means the result should be sorted in alphabetically ascending order, whereas - means the result list should be sorted in alphabetically descending order.
Here's how sorting works

## Filtering
Filters are supplied as string arrays. The string array indicates which fields the resulting list should contain, if they are available. This is best illustrated using some examples:

    ['Artist']
    ['Artist', 'DisplayName', 'Path']
    ['*']
    ['Resources', 'URL']

The first example will only include the Artist field in the result. The second shows how we can select several fields. The third example shows the wildcard selection,*, which is used to select all fields.

The last example can be used to illustrate that key names are selected recursively in the resulting structure. The Resources field contains a list of fields, where URL is one of the fields. Specifying URL in the filter list will include "Resources/URL" even though the URL field is nested inside the Resources field.
