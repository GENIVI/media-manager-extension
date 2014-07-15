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

extension.setMessageListener(function(msg) {
    var obj = JSON.parse(msg);
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

Indexer.prototype.getDatabasePath = function(cb) {
    return this.jsonRPC.request('getDatabasePath', [], cb);
};

Indexer.prototype.startIndexing = function (cb) {
    return this.jsonRPC.request('startIndexing',[], cb);
};

Indexer.prototype.stopIndexing = function(cb) {
    return this.jsonRPC.request('stopIndexing',[], cb);
};

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

Browser.prototype.createReference = function (container, object, cb) {
    if (!('Path' in container)) {
        return -1;
    }

    var path = container['Path'];

    return this.jsonRPC.request('createReference', [path, object], cb);
};

/* Player API */
function Player () {
    this.jsonRPC = jsonRPCInstance;
};

Player.prototype.openUri = function (uri, cb) {
    return this.jsonRPC.request('openUri', [uri], cb);
};

Player.prototype.pause = function (cb) {
    return this.jsonRPC.request('pause', [], cb);
};

Player.prototype.play = function (cb) {
    return this.jsonRPC.request('play', [], cb);
};

Player.prototype.playPause = function (cb) {
    return this.jsonRPC.request('playPause', [], cb);
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
