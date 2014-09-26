 /*
  * Copyright (C) 2014, Jaguar Land Rover
  *
  * Author: Jonatan Palsson <jonatan.palsson@pelagicore.com>
  *
  * This file is part of the GENIVI Media Manager Proof-of-Concept
  * For further information, see http://genivi.org/
  *
  * This Source Code Form is subject to the terms of the Mozilla Public
  * License, v. 2.0. If a copy of the MPL was not distributed with this
  * file, You can obtain one at http://mozilla.org/MPL/2.0/.
  */

var callbackMap = {};
var jsonRPCInstance = new jsonRPC();
var notificationHandler = null;

function handleNotification (obj) {
    console.log ('Received notification: ' + JSON.stringify(obj));
    var method = obj['method'];
    var params = obj['params'];

    if (notificationHandler) {
        notificationHandler (method, params);
    } else {
        console.log ('No notification handler registered!');
    }
};

extension.setMessageListener(function(msg) {
    var obj = JSON.parse(msg);

    if (!('id' in obj)) {
        handleNotification(obj);
        return;
    }

    var id = obj.id.toString();
    var error = null;

    if (!(id in callbackMap)) {
        console.log ('ERROR! Callback ID '+id+' not recognized');
        return;
    }

    if ('error' in obj) {
        error = obj.error;
        obj.result = null;
    }

    var echoListener = callbackMap[id];

    if (echoListener != undefined)
        echoListener(obj.result, error);
    else
        console.log ('Warning: No callback set for this call');

   delete callbackMap[id];
});

/**
 * @global
 * @class Indexer
 * @description Entry point to indexer-related functionality
 */
function Indexer () {
    this.jsonRPC = jsonRPCInstance;
};
/** 
 *  @global
 *  @description: Get the path of the database file currently in use by LMS.
 *  @param cb Callback function with the following parameters:
 *  @param {string} cb the database path
 *  @param {string} cb any potential error
 */
Indexer.prototype.getDatabasePath = function(cb) {
    return this.jsonRPC.request('getDatabasePath', [], cb);
};

/**
 *  @global
 *  @description Start the indexing process in LMS. Starting indexing while
 *               indexing is already commencing is an error.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb any potential error
 */
Indexer.prototype.startIndexing = function (cb) {
    return this.jsonRPC.request('startIndexing',[], cb);
};

/**
 *  @global
 *  @description Stop the indexing process. It is an error to stop an already
 *               stopped indexing.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb any potential error
 */
Indexer.prototype.stopIndexing = function(cb) {
    return this.jsonRPC.request('stopIndexing',[], cb);
};

/**
 *  @global
 *  @description Get the indexer status. RUNNING means the indexer is indexing.
 *               STOPPED means the indexer has been stopped, IDLE means the
 *               indexer is idling, and waiting for events to trigger indexing
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb one of: 'RUNNING', 'STOPPED', 'IDLE'
 *  @param {string} cb any potential error
*/
Indexer.prototype.getIndexerStatus = function(cb) {
    return this.jsonRPC.request('getIndexerStatus',[], cb);
};

/** 
 *  @global
 *  @class Browser
 *  @description Entry point to browser-related functionality
 */
function Browser () {
    this.jsonRPC = jsonRPCInstance;
};

/**
 *  @global
 *  @description Convenience function to get the root of a Media Manager
 *               server. This object is used to list the top level containers,
 *               and once these has been retreived, they will contain further
 *               paths which can be explored.
 *  @returns Identifier of root object which can be used in {@link listItems},
 *           {@link listContainers}, etc.
 */
Browser.prototype.RootObject = function (id) {
    id = id || '1';
    return {
        'DisplayName': 'Root',
        'Path': '/com/intel/dLeynaServer/server/' + id
    };
};

/**
 * @global
 * @description Get a list of media manager sources, these are typically DLNA
 * servers. The items in the list should be regarded as tokens, and should not
 * be parsed or manipulated as their structure may change. If any media manager
 * is acceptable (there is most likely only one) - pick the first element of
 * this list.
 *
 * The tokens from this function are used when first issuing a call to the
 * other functions of the {@link Browse} object, as the inital token.
 *  @param cb Callback function with the following parameters:
 *  @param cb a list of source tokens, which can be regarded as containers,
 *        and used as containers with the rest of the functions of the {@link
 *        Browser} object.
 *  @param cb any potential error
 */
Browser.prototype.discoverMediaManagers = function (cb) {
    return this.jsonRPC.request('discoverMediaManagers', [], cb);
};

/**
 *  @description  List the containers under a given container. If no containers
 *  are available (if there are only items, for instance), an empty list is
 *  returned.
 *  @param {string} container: A path token found either by using {@link discoverMediaManagers},
 *      or found as the value of the Path key in a container object. Container
 *      objects are found using listContainers.
 *  @param {int} offset: Offset at which to start producing results, used when paging
 *  @param {int} count: The number of results to return, used when paging
 *  @param {string[]} filter: List of keys to include in container object, ['*'] means all
 *  @param cb Callback function with the following parameters:
 *  @param cb a list of containers contained below the supplied path. The
 *         Path parameter of these containers can be used to further traverse
 *         the container structure using listContainers, or used with other
 *         functions of the Browser object.
 *  @param cb any potential error
 */
Browser.prototype.listContainers = function (container, offset, count, filter, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listContainers',
                                [path, offset, count, filter],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/**
 *  @description List the containers under a given container. Allows sorting.
 *  Sorting has the same properties as in dLeyna (and Rygel); prefix a key with
 *  + or -, where + will give you descending results, and - will produce
 *  descending results. As an example; sort by Artist ascending: "-Artist".
 *  @param {string} container: A path token found either by using discoverMediaManagers,
 *      or found as the value of the Path key in a container object.
 *  @param {int} offset: Offset at which to start producing results, used when paging
 *  @param {int} count: The number of results to return, used when paging
 *  @param {string[]} filter: List of keys to include in container object, ['*'] means all
 *  @param {string} sortKey: Specify a key to sort on, see function description for format
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object} cb a list of containers contained below the supplied path. The
 *         Path parameter of these containers can be used to further traverse
 *         the container structure using listContainers, or used with other
 *         functions of the Browser object. List is sorted on sortKey.
 *  @param {string} cb any potential error
 */
Browser.prototype.listContainersEx = function (container, offset, count, filter, sortKey, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listContainersEx',
                                [path, offset, count, filter, sortKey],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/** 
 *  @description List the items under a container. Will produce an empty list
 *  if there are no items in the container (for instance if there are only
 *  containers)
 *  @param {string} container: The container to list items below
 *  @param {int} offset: The offset to start listing items from, used when paging
 *  @param {int} count: The number of results to produce, used when paging
 *  @param {string[]} filter: List of keys to include in the items, ['*'] means all
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} cb a JSON list of items containing the keys specified in filter
 *  @param {string} cb any potential error
 */
Browser.prototype.listItems = function (container, offset, count, filter, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listItems', [path, offset, count, filter],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/** @description List the items under a container. Allows sorting. See {@link
 *  listContainersEx} for an explanation on sort keys
 *  @param {string} container: The container to list items below
 *  @param {int} offset: The offset to start listing items from, used when paging
 *  @param {int} count: The number of results to produce, used when paging
 *  @param {string[]} filter: List of keys to include in the items, ['*'] means all
 *  @param {string} sortKey: Specify a key to sort on
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} a JSON list of items containing the keys specified in filter,
 *         sorted on sortKey.
 *  @param {string} any potential error
 */
Browser.prototype.listItemsEx = function (container, offset, count, filter, sortKey, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listItemsEx', [path, offset, count, filter, sortKey],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/** @description List items and containers under a container. This should
 *  always return a result when provided with a valid container, as Rygel will
 *  hide empty containers.
 *  @param {string} container: The container to children below
 *  @param {int} offset: The offset to start listing children from, used when paging
 *  @param {int} count: The number of children to list, used when paging
 *  @param {string[]} filter: The keys to include in each child
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} a list of children, containing the keys specified in filter
 *  @param {string} any potential error
 */
Browser.prototype.listChildren = function (container, offset, count, filter, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listChildren', [path, offset, count, filter],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/** @description List items and containers under a container. Allows sorting.
 *  See listChildren, and listContainersEx for an explanation on sorting.
 *  @param {string} container: The container to list children below
 *  @param {int} offset: The offset to start listing children from, used when paging
 *  @param {int} count: The number of children to list, used when paging
 *  @param {string[]} filter: The keys to include in each child
 *  @param {string} sortKey: Specify a key to sort on
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} cb a list of children, containing the keys specified in filter,
 *        sorted on sortKey
 *  @param {string} cb any potential error
 */
Browser.prototype.listChildrenEx = function (container, offset, count, filter, sortKey, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listChildrenEx', [path, offset, count, filter, sortKey],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/** @description Search a container, and produce a list of items and containers
 *  matching the search. The query language is defined in the MediaServer2 spec
 *  (https://wiki.gnome.org/Projects/Rygel/MediaServer2Spec). Field names are
 *  not specified in the spec (since they are implementation specific). In this
 *  API, we assume dLeyna field names are used. The dLeyna field names are the
 *  keys found in the result objects when running listItems or listContainers.
 *  An example query looks like this: 'TypeEx derivedFrom
 *  "container.music.musicAlbum" and Artist contains "Madonna"', this query will
 *  produce all MusicAlbum containers (albums) where the Artist field contains
 *  Madonna. Note that the query language assumes double quotes and single
 *  quotes for strings are not allowed.
 *  @param {string} container: The container to search in
 *  @param {int} offset: Offset in results list to start listing from, used when paging
 *  @param {int} count: Number of results to produce
 *  @param {string[]} filter: Keys to include in result objects
 *  @param {string} query: dLeyna query
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} cb a list of result objects, containing the keys specified
 *  @param {string} cb any potential error
 */
Browser.prototype.searchObjects = function (container, offset, count, filter, query, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('searchObjects', [path, offset, count, filter, query],
                                function (msg, error) {
                                    console.log ('In callback, message: ' + msg);
                                    cb (JSON.parse(msg), error);
                                });
};

/** @description Search a container, and produce a list of items and containers
 *  matching the search, allows sorting. See searchObjects for an explanation on
 *  searching, and listContainersEx for an explanation on sorting
 *  @param {string} container: The container to search in
 *  @param {int} offset: Offset in results list to start listing from, used when paging
 *  @param {int} count: Number of results to produce
 *  @param {string[]} filter: Keys to include in result objects
 *  @param {string} sortKey: Specify a key to sort on
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} cb a list of result objects, containing the keys specified,
 *         sorted according to sortKey
 *  @param {string} cb any potential error
 */
Browser.prototype.searchObjectsEx = function (container, offset, count, filter, sortKey, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('searchObjectsEx', [path, offset, count, filter, sortKey],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/**
 *  @description Get a list of all the initial letters of the DisplayName field
 *  of all children in a container. This function is not implemented.
 *  @param {string} container: The container to list indexes in
 *  @param {int} count: The number of indexes to produce
 *  @param {string[]} filter: The initial letters to include ['*'] means all
 *  @param {string} sortKey: Key to sort on when building index list
 *  @param {function} cb Callback function with the following parameters:
 *  @param {Object[]} cb a list of tuples of initial letters and their first observed
 *        position when sorting according to sortKey
 *  @param {string} cb any potential error
 */
Browser.prototype.listIndexes = function (container, count, filter, sortKey, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('listIndexes', [path, offset, count, filter, sortKey],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};
/* @description Create a reference to an object in a container. The container
 * needs to be writable. This typically means the container is a playlist. This
 * is used to "add" objects (typically MusicItems) to containers, thereby
 * creating playlists.
 *  @param {string} container: The container to create a reference in
 *  @param {Object} object: The object to create reference to. Needs to
 *  contain a Path key to identify the object.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb the newly created reference
 *  @param {string} cb any potential error
 */
Browser.prototype.createReference = function (container, object, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('createReference', [path, object], cb);
};

/* @description Create a container within another container. This requires the parent
 * container to be writable, which typically means it is a playlist. This is
 * used to created nested playlists.
 *  @param {string} container: Parent container to place new container in
 *  @param {Object} object: The displayname of the new container
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb a reference to the newly created container
 *  @param {string} cb any potential error
 */
Browser.prototype.createContainer = function (container, object, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('createContainer', [path, object], cb);
};

/**
 * @global
 * @class Player
 * @description Entry point to the player-related functionality
 */
function Player () {
    this.jsonRPC = jsonRPCInstance;
};

/**
 *  @description Open a URI for playback. The URI needs to be accessible to the
 *  player. The file indicated by the URI will be opened for playback
 *  immediately. Using this function will not enqueue the file in question, but
 *  bypass the queue entirely.
 *  @param uri: A URI accessible to the player, such as a file:// URI
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.openUri = function (uri, cb) {
    return this.jsonRPC.request('openUri', [uri], cb);
};

/** 
 *  @description Pause playback of the currently playing media
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.pause = function (cb) {
    return this.jsonRPC.request('pause', [], cb);
};

/**
 *  @description Play the currently selected media
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.play = function (cb) {
    return this.jsonRPC.request('play', [], cb);
};

/** @description Stop the currently selected media. This will behave like a
 * pause, and not actually clear any buffers or other information in GStreamer.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.stop = function (cb) {
    return this.jsonRPC.request('stop', [], cb);
};


/** @description Play or pause the currently selected media, see the play and
 *  pause functions.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
*/
Player.prototype.playPause = function (cb) {
    return this.jsonRPC.request('playPause', [], cb);
};

/** @description Go to the next track in the play queue. It is not an error to
 *  skip past the play queue, however no action will be taken.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.next = function (cb) {
    return this.jsonRPC.request('next', [], cb);
};

/** @description Open a playlist and use it as play queue. The playlist is
 *  indicated by the value of a Path key in any container. The container being
 *  used as a play list has to contain items, and only direct child items will
 *  be played. This function will replace the current play queue with the
 *  contents of the supplied container (as opposed to appending).
 *  @param {string} playlist: A container to use as playlist. This is the Path
 *  value of any container.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
*/
Player.prototype.openPlaylist = function (playlist, cb) {
    return this.jsonRPC.request('openPlaylist', [playlist], cb);
};

/** @description Go to the previous track in the play queue. It is not an error
 *  to step out of the play queue, but no action will be taken.
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
*/
Player.prototype.previous = function (cb) {
    return this.jsonRPC.request('previous', [], cb);
};

/** @description Seek backwards or forwards in the current track. It is not an
 *  error to seek outside a track, but no action will be taken.
 *  @param {int} pos: Relative seek amount in microseconds (negative seek means seek backwards)
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.seek = function (pos, cb) {
    return this.jsonRPC.request('seek', [pos], cb);
};

/* @description Set the play position to an absolute number. This is equivalent
 *  to an absolute seek.
 *  @param {int} pos: Absolute position in microseconds
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.setPosition = function (pos, cb) {
    return this.jsonRPC.request('setPosition', [pos], cb);
};

/** Not implemented, this is should be handled by the system */
Player.prototype.setMuted = function (muted, cb) {
    return this.jsonRPC.request('setMuted', [muted], cb);
};

/** Not implemented, this is should be handled by the system */
Player.prototype.getMuted = function (cb) {
    return this.jsonRPC.request('getMuted', [], cb);
};

/** @description Set the 'shuffled' property. When tracks are shuffled, the
 *  CurrentTrack signal will indicate tracks in a random order, as opposed to a
 *  sequential order.
 *  @param {boolean} true to enable shuffle, false to disable
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
*/
Player.prototype.setShuffled = function (shuffled, cb) {
    return this.jsonRPC.request('setShuffled', [shuffled], cb);
};

/** @description Get the 'shuffled' property
 *  @param {function} cb Callback function with the following parameters:
 *  @param {bool} cb true when shuffled, false when not
 *  @param {string} cb any potential error
 */
Player.prototype.getShuffled = function (cb) {
    return this.jsonRPC.request('getShuffled', [], cb);
};

/** @description Set the 'repeated' property. REPEAT will repeat all tracks in order, NO_REPEAT will disable repeat, REPEAT_SINGLE will repeat the current track indefinitely
 *  @param {string} repeated 'REPEAT', 'NO_REPEAT', 'REPEAT_SINGLE'
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
*/
Player.prototype.setRepeated = function (repeat, cb) {
    return this.jsonRPC.request('setRepeated', [repeat], cb);
};

/** Get the 'repeated' property
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb 'REPEAT', 'NO_REPEAT', or 'REPEAT_SINGLE'
 *  @param {string} cb is any potential error
 */
Player.prototype.getRepeated = function (cb) {
    return this.jsonRPC.request('getRepeated', [], cb);
};

/**
 *  @description Set the 'rate' property. This indicates the rate at which
 *  music should be played back. The available rates depend on the backend
 *  renderer and can not be determined by the API.
 *  @param {int} rate: 1, 2, 4, 8 or 16 are supported by the API, byt may not be supported by the backend renderer
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb is any potential error
 */
Player.prototype.setRate = function (rate, cb) {
    return this.jsonRPC.request('setRate', [rate], cb);
};

/** Get the 'rate' property
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb 1, 2, 4, 8 or 16
 *  @param {string} cb is any potential error
*/
Player.prototype.getRate = function (cb) {
    return this.jsonRPC.request('getRate', [], cb);
};

/** Not implemented, this is handled by the system */
Player.prototype.getVolume = function (cb) {
    return this.jsonRPC.request('getVolume', [], cb);
};

/** Not implemented, this is handled by the system */
Player.prototype.setVolume = function (volume, cb) {
    return this.jsonRPC.request('setVolume', [volume], cb);
};

/** Get the 'CanGoNext' property, indicating if a call to Next is expected to
 * succeed
 *  @param {function} cb Callback function with the following parameters:
 *  @param {bool} cb true/false
 *  @param {string} cb is any potential error
*/
Player.prototype.getCanGoNext = function (cb) {
    return this.jsonRPC.request('getCanGoNext', [], cb);
};

/** Get the 'CanGoPrevious' property, indicating if a call to Previous is expected to
 * succeed
 *  @param {function} cb Callback function with the following parameters:
 *  @param {bool} cb true/false
 *  @param {string} cb is any potential error
*/
Player.prototype.getCanGoPrevious = function (cb) {
    return this.jsonRPC.request('getCanGoPrevious', [], cb);
};

/** Get the 'CanPause' property, indicating if a call to Pause is expected to
 * succeed
 *  @param {function} cb Callback function with the following parameters:
 *  @param {bool} cb true/false
 *  @param {string} cb is any potential error
*/
Player.prototype.getCanPause = function (cb) {
    return this.jsonRPC.request('getCanPause', [], cb);
};

/** Get the 'CanPlay' property, indicating if a call to Play is expected to
 * succeed
 *  @param {function} cb Callback function with the following parameters:
 *  @param {bool} cb true/false
 *  @param {string} cb is any potential error
*/
Player.prototype.getCanPlay = function (cb) {
    return this.jsonRPC.request('getCanPlay', [], cb);
};

/** Get the 'CanSeek' property, indicating if a call to Seek is expected to
 * succeed
 *  @param {function} cb Callback function with the following parameters:
 *  @param {bool} cb true/false
 *  @param {string} cb is any potential error
*/
Player.prototype.getCanSeek = function (cb) {
    return this.jsonRPC.request('getCanSeek', [], cb);
};

/** Get the index of the current track in the current play list
 *  @param {function} cb Callback function with the following parameters:
 *  @param {int} cb index of currently playing track
 *  @param {string} cb is any potential error
 *  @see Player#getCurrentPlayQueue
*/
Player.prototype.getCurrentTrack = function (cb) {
    return this.jsonRPC.request('getCurrentTrack', [], cb);
};

/** Get the current playback status
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb "playing" or "paused"
 *  @param {string} cb is any potential error
*/
Player.prototype.getPlaybackStatus = function (cb) {
    return this.jsonRPC.request('getPlaybackStatus', [], cb);
};

/** Get the current playback position
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb the position in the current song, in microseconds
 *  @param {string} cb is any potential error
*/
Player.prototype.getPosition = function (cb) {
    return this.jsonRPC.request('getPosition', [], cb);
};

/** Get the duration of the current track in microseconds
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb The duration of the current song, in microseconds
 *  @param {string} cb is any potential error
*/
Player.prototype.getDuration = function (cb) {
    return this.jsonRPC.request('getDuration', [], cb);
};

/** Get the current play queue, as a list of items
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb any potential error
*/
Player.prototype.getCurrentPlayQueue = function (cb) {
    return this.jsonRPC.request('getCurrentPlayQueue', [],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};

/** Enqueue a URI in the play queue
    @param uri: The URI to enqueue in the play queue
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb any potential error
*/
Player.prototype.enqueueUri = function (uri, cb) {
    return this.jsonRPC.request('enqueueUri', [uri],
                                function (msg, error) {
                                    cb (msg, error);
                                });
};

/** Dequeue the track with the specified index from the play queue
    @param idx: Index of track to dequeue
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb any potential error
*/
Player.prototype.dequeueIndex = function (idx, cb) {
    return this.jsonRPC.request('dequeueIndex', [idx],
                                function (msg, error) {
                                    cb (msg, error);
                                });
};

/** Empty the play queue
 *  @param {function} cb Callback function with the following parameters:
 *  @param {string} cb Never set (void return value)
 *  @param {string} cb any potential error
*/
Player.prototype.emptyPlayQueue = function (cb) {
    return this.jsonRPC.request('dequeueAll', [],
                                function (msg, error) {
                                    cb (msg, error);
                                });
};

function jsonRPC () {
    this.idCtr = 1;
}

jsonRPC.prototype.request = function(fname, params, cb) {
    var req = {jsonrpc: '2.0',
               method: fname,
               id: this.idCtr.toString()};

    if (params != [])
        req.params = params;

    this.idCtr++;
    if (req.id in callbackMap) {
        console.log ('ERROR! Callback ID already used!');
        return null;
    }
    callbackMap[req.id] = cb;
    extension.postMessage(JSON.stringify(req));
};

exports.Indexer = Indexer;
exports.Browser = Browser;
exports.Player = Player;

/* Used to register a notification handler, which will receive events such as
 * playback status changed, etc.
    @param cb: function (string method, string parameter)
        - msg is the property which has changed
        - parameter is the new value of the property
*/
exports.registerNotificationHandler = function (cb) {
    notificationHandler = cb;
}

