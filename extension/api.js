 /**
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

/* Indexer API */
function Indexer () {
    this.jsonRPC = jsonRPCInstance;
};
/* Get the path of the database file currently in use
   @param cb: function(string msg, string error) - message is the database path
                                                 - error is any potential error
*/
Indexer.prototype.getDatabasePath = function(cb) {
    return this.jsonRPC.request('getDatabasePath', [], cb);
};

/* Start the indexing process
   @param cb: function(string msg, string error) - msg is never set
                                                 - error is any potential error
*/
Indexer.prototype.startIndexing = function (cb) {
    return this.jsonRPC.request('startIndexing',[], cb);
};

/* Stop the indexing process
   @param cb: function(string msg, string error) - msg is never set
                                                 - error is any potential error
*/
Indexer.prototype.stopIndexing = function(cb) {
    return this.jsonRPC.request('stopIndexing',[], cb);
};

/* Get the indexer status
   @param cb: function(string msg, string error) - msg is either 'RUNNING', 'STOPPED', or 'IDLE'
                                                 - error is any potential error
*/
Indexer.prototype.getIndexerStatus = function(cb) {
    return this.jsonRPC.request('getIndexerStatus',[], cb);
};

/* Browser API */
function Browser () {
    this.jsonRPC = jsonRPCInstance;
};

Browser.prototype.RootObject = function (id) {
    id = id || '1';
    return {
        'DisplayName': 'Root',
        'Path': '/com/intel/dLeynaServer/server/' + id
    };
};

/* Get a list of media manager sources, these are typically DLNA servers. The
 * items in the list should be regarded as tokens, and should not be parsed or
 * manipulated as their structure may change. If any media manager is
 * acceptable (there is most likely only one) - pick the first element of this
 * list.
 *
 * The tokens from this function are used when first issuing a call to the
 * other functions of the Browse object, as the inital token.
   @param cb: function(JSON list msg, string error)
        - msg is a list of source tokens, which can be regarded as containers,
          and used as containers with the rest of the functions of the Browser
          object.
        - error is any potential error
*/
Browser.prototype.discoverMediaManagers = function (cb) {
    return this.jsonRPC.request('discoverMediaManagers', [], cb);
};

/* List the containers under a given container.
    @param container: A path token found either by using discoverMediaManagers,
        or found as the value of the Path key in a container object. Container
        objects are found using listContainers.
    @param offset: Offset at which to start producing results, used when paging
    @param count: The number of results to return, used when paging
    @param filter: List of keys to include in container object, ['*'] means all
    @param cb: function(JSON list msg, string error)
         - msg is a list of containers contained below the supplied path. The
           Path parameter of these containers can be used to further traverse
           the container structure using listContainers, or used with other
           functions of the Browser object.
         - error is any potential error
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

/* List the containers under a given container. Allows sorting.
    @param container: A path token found either by using discoverMediaManagers,
        or found as the value of the Path key in a container object.
    @param offset: Offset at which to start producing results, used when paging
    @param count: The number of results to return, used when paging
    @param filter: List of keys to include in container object, ['*'] means all
    @param sortKey: Specify a key to sort on
    @param cb: function(string msg, string error)
         - msg is a list of containers contained below the supplied path. The
           Path parameter of these containers can be used to further traverse
           the container structure using listContainers, or used with other
           functions of the Browser object. List is sorted on sortKey.
         - error is any potential error
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

/* List the items under a container
    @param container: The container to list items below
    @param offset: The offset to start listing items from, used when paging
    @param count: The number of results to produce, used when paging
    @param filter: List of keys to include in the items, ['*'] means all
    @param cb: function (JSON list msg, string error)
         - msg is a JSON list of items containing the keys specified in filter
         - error is any potential error
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

/* List the items under a container. Allows sorting.
    @param container: The container to list items below
    @param offset: The offset to start listing items from, used when paging
    @param count: The number of results to produce, used when paging
    @param filter: List of keys to include in the items, ['*'] means all
    @param sortKey: Specify a key to sort on
    @param cb: function (JSON list msg, string error)
         - msg is a JSON list of items containing the keys specified in filter,
           sorted on sortKey.
         - error is any potential error
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

/* List items and containers under a container.
    @param container: The container to children below
    @param offset: The offset to start listing children from, used when paging
    @param count: The number of children to list, used when paging
    @param filter: The keys to include in each child
    @param cb: function(JSON list msg, string error)
        - msg is a list of children, containing the keys specified in filter
        - error is any potential error
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

/* List items and containers under a container. Allows sorting.
    @param container: The container to list children below
    @param offset: The offset to start listing children from, used when paging
    @param count: The number of children to list, used when paging
    @param filter: The keys to include in each child
    @param sortKey: Specify a key to sort on
    @param cb: function(JSON list msg, string error)
        - msg is a list of children, containing the keys specified in filter,
          sorted on sortKey
        - error is any potential error
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

/* Search a container, and produce a list of items and containers matching the
 * search
    @param container: The container to search in
    @param offset: Offset in results list to start listing from, used when paging
    @param count: Number of results to produce
    @param filter: Keys to include in result objects
    @param query: dLeyna query
    @param cb: function (JSON list msg, string error)
         -msg is a list of result objects, containing the keys specified
         -error is any potential error
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

/* Search a container, and produce a list of items and containers matching the
 * search, allows sorting.
    @param container: The container to search in
    @param offset: Offset in results list to start listing from, used when paging
    @param count: Number of results to produce
    @param filter: Keys to include in result objects
    @param sortKey: Specify a key to sort on
    @param cb: function (JSON list msg, string error)
         - msg is a list of result objects, containing the keys specified,
           sorted according to sortKey
         - error is any potential error
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

/* Get a list of all the initial letters and their index in the given container
    @param container: The container to list indexes in
    @param count: The number of indexes to produce
    @param filter: The initial letters to include ['*'] means all
    @param sortKey: Key to sort on when building index list
    @param cb function(JSON list msg, string error)
        - msg is a list of tuples of initial letters and their first observed
          position when sorting according to sortKey
        - error is any potential error
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
/* Create a reference to an object in a container. The container needs to be
 * writable. This typically means the container is a playlist.
    @param container: The container to create a reference in
    @param object: The object to create reference to
    @param cb function(string msg, string error)
        - msg is the newly created reference
        - error is any potential error
*/
Browser.prototype.createReference = function (container, object, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('createReference', [path, object], cb);
};

/* Create a container within another container. This requires the parent
 * container to be writable, which typically means it is a playlist.
    @param container: Parent container to place new container in
    @param object: The displayname of the new container
    @param cb: function(string msg, string error)
        - msg is a reference to the newly created container
        - error is any potential error
*/
Browser.prototype.createContainer = function (container, object, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('createContainer', [path, object], cb);
};

/* Player API */
function Player () {
    this.jsonRPC = jsonRPCInstance;
};

/* Open a URI for playback. The URI needs to be accessible to the player.
    @param uri: A URI accessible to the player, such as a file:// URI
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.openUri = function (uri, cb) {
    return this.jsonRPC.request('openUri', [uri], cb);
};

/* Pause playback of the currently playing media
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.pause = function (cb) {
    return this.jsonRPC.request('pause', [], cb);
};

/* Play the currently selected media
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.play = function (cb) {
    return this.jsonRPC.request('play', [], cb);
};

/* Stop the currently selected media
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.stop = function (cb) {
    return this.jsonRPC.request('stop', [], cb);
};


/* Play or pause the currently selected media
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.playPause = function (cb) {
    return this.jsonRPC.request('playPause', [], cb);
};

/* Go to the next track in the play queue
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.next = function (cb) {
    return this.jsonRPC.request('next', [], cb);
};

/* Open a playlist and use it as play queue
    @param playlist: A container to use as playlist
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.openPlaylist = function (playlist, cb) {
    return this.jsonRPC.request('openPlaylist', [playlist], cb);
};

/* Go to the previous track in the play queue
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.previous = function (cb) {
    return this.jsonRPC.request('previous', [], cb);
};

/* Seek backwards or forwards in the current track
    @param pos: Relative seek amount in microseconds
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.seek = function (pos, cb) {
    return this.jsonRPC.request('seek', [pos], cb);
};

/* Set the play position to an absolute number
    @param pos: Absolute position in microseconds
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.setPosition = function (pos, cb) {
    return this.jsonRPC.request('setPosition', [pos], cb);
};

/* Set the 'muted' property
    @param muted: true/false
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.setMuted = function (muted, cb) {
    return this.jsonRPC.request('setMuted', [muted], cb);
};

/* Get the 'muted' property
    @param cb: function (bool msg, string error)
        - msg is true or false
        - error is any potential error
*/
Player.prototype.getMuted = function (cb) {
    return this.jsonRPC.request('getMuted', [], cb);
};

/* Set the 'shuffled' property
    @param shuffled: true/false
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.setShuffled = function (shuffled, cb) {
    return this.jsonRPC.request('setShuffled', [shuffled], cb);
};

/* Get the 'shuffled' property
    @param cb: function (bool msg, string error)
        - msg is true or false
        - error is any potential error
*/
Player.prototype.getShuffled = function (cb) {
    return this.jsonRPC.request('getShuffled', [], cb);
};

/* Set the 'repeated' property
    @param repeated: 'REPEAT', 'NO_REPEAT', 'REPEAT_SINGLE'
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.setRepeated = function (repeat, cb) {
    return this.jsonRPC.request('setRepeated', [repeat], cb);
};

/* Get the 'repeated' property
    @param cb: function (bool msg, string error)
        - msg is true or false
        - error is any potential error
*/
Player.prototype.getRepeated = function (cb) {
    return this.jsonRPC.request('getRepeated', [], cb);
};

/* Set the 'rate' property
    @param rate: 1, 2, 4, 8 or 16
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.setRate = function (rate, cb) {
    return this.jsonRPC.request('setRate', [rate], cb);
};

/* Get the 'rate' property
    @param cb: function (int msg, string error)
        - msg is 1, 2, 4, 8 or 16
        - error is any potential error
*/
Player.prototype.getRate = function (cb) {
    return this.jsonRPC.request('getRate', [], cb);
};

/* Get the 'volume' property
    @param cb: function (double msg, string error)
        - msg is between 0.0 and 1.0
        - error is any potential error
*/
Player.prototype.getVolume = function (cb) {
    return this.jsonRPC.request('getVolume', [], cb);
};

/* Set the 'volume' property
    @param volume: Value between 0.0 and 1.0
    @param cb: function (string msg, string error)
        - msg is never set
        - error is any potential error
*/
Player.prototype.setVolume = function (volume, cb) {
    return this.jsonRPC.request('setVolume', [volume], cb);
};

/* Get the 'CanGoNext' property, indicating if a call to Next is expected to
 * succeed
    @param cb: function (bool msg, string error)
        - msg is true/false
        - error is any potential error
*/
Player.prototype.getCanGoNext = function (cb) {
    return this.jsonRPC.request('getCanGoNext', [], cb);
};

/* Get the 'CanGoPrevious' property, indicating if a call to Previous is expected to
 * succeed
    @param cb: function (bool msg, string error)
        - msg is true/false
        - error is any potential error
*/
Player.prototype.getCanGoPrevious = function (cb) {
    return this.jsonRPC.request('getCanGoPrevious', [], cb);
};

/* Get the 'CanPause' property, indicating if a call to Pause is expected to
 * succeed
    @param cb: function (bool msg, string error)
        - msg is true/false
        - error is any potential error
*/
Player.prototype.getCanPause = function (cb) {
    return this.jsonRPC.request('getCanPause', [], cb);
};

/* Get the 'CanPlay' property, indicating if a call to Play is expected to
 * succeed
    @param cb: function (bool msg, string error)
        - msg is true/false
        - error is any potential error
*/
Player.prototype.getCanPlay = function (cb) {
    return this.jsonRPC.request('getCanPlay', [], cb);
};

/* Get the 'CanSeek' property, indicating if a call to Seek is expected to
 * succeed
    @param cb: function (bool msg, string error)
        - msg is true/false
        - error is any potential error
*/
Player.prototype.getCanSeek = function (cb) {
    return this.jsonRPC.request('getCanSeek', [], cb);
};

/* Get the index of the current track in the current play list
    @param cb: function (int msg, string error)
        - msg is the index of the current track
        - error is any potential error
*/
Player.prototype.getCurrentTrack = function (cb) {
    return this.jsonRPC.request('getCurrentTrack', [], cb);
};

/* Get the current playback status
    @param cb: function (string msg, string error)
        - msg is 'playing' or 'paused'
        - error is any potential error
*/
Player.prototype.getPlaybackStatus = function (cb) {
    return this.jsonRPC.request('getPlaybackStatus', [], cb);
};

/* Get the current playback position
    @param cb: function (int64 msg, string error)
        - msg is the position in the current song, in microseconds
        - error is any potential error
*/
Player.prototype.getPosition = function (cb) {
    return this.jsonRPC.request('getPosition', [], cb);
};


Player.prototype.getCurrentPlayQueue = function (cb) {
    return this.jsonRPC.request('getCurrentPlayQueue', [],
                                function (msg, error) {
                                    cb (JSON.parse(msg), error);
                                });
};
Player.prototype.enqueueUri = function (uri, cb) {
    return this.jsonRPC.request('enqueueUri', [uri],
                                function (msg, error) {
                                    cb (msg, error);
                                });
};
Player.prototype.dequeueIndex = function (cb) {
    return this.jsonRPC.request('dequeueIndex', [],
                                function (msg, error) {
                                    cb (msg, error);
                                });
};

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

