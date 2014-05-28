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

    echoListener(obj.result, error);

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

Browser.prototype.listContainers = function (path, offset, count, filter, cb) {
    return this.jsonRPC.request('listContainers',
                                [path, offset, count, filter],
                                cb);
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
