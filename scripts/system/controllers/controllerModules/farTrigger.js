"use strict";

//  farTrigger.js
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html


/* global Script, Controller, LaserPointers, RayPick, RIGHT_HAND, LEFT_HAND, MyAvatar, getGrabPointSphereOffset,
   makeRunningValues, Entities, enableDispatcherModule, disableDispatcherModule, makeDispatcherModuleParameters,
   PICK_MAX_DISTANCE, COLORS_GRAB_SEARCHING_HALF_SQUEEZE, COLORS_GRAB_SEARCHING_FULL_SQUEEZE, COLORS_GRAB_DISTANCE_HOLD,
   DEFAULT_SEARCH_SPHERE_DISTANCE, getGrabbableData, makeLaserParams
*/

Script.include("/~/system/libraries/controllerDispatcherUtils.js");
Script.include("/~/system/libraries/controllers.js");

(function() {
    function entityWantsNearTrigger(props) {
        var grabbableData = getGrabbableData(props);
        return grabbableData.triggerable || grabbableData.wantsTrigger;
    }

    function FarTriggerEntity(hand) {
        this.hand = hand;
        this.targetEntityID = null;
        this.grabbing = false;
        this.previousParentID = {};
        this.previousParentJointIndex = {};
        this.previouslyUnhooked = {};

        this.parameters = makeDispatcherModuleParameters(
            520,
            this.hand === RIGHT_HAND ? ["rightHand"] : ["leftHand"],
            [],
            100,
            makeLaserParams(this.hand, false));

        this.getTargetProps = function (controllerData) {
            // nearbyEntityProperties is already sorted by length from controller
            var targetEntity = controllerData.rayPicks[this.hand].objectID;
            if (targetEntity) {
                var targetProperties = Entities.getEntityProperties(targetEntity);
                if (entityWantsNearTrigger(targetProperties)) {
                    return targetProperties;
                }
            }
            return null;
        };

        this.startFarTrigger = function (controllerData) {
            var args = [this.hand === RIGHT_HAND ? "right" : "left", MyAvatar.sessionUUID];
            Entities.callEntityMethod(this.targetEntityID, "startFarTrigger", args);
        };

        this.continueFarTrigger = function (controllerData) {
            var args = [this.hand === RIGHT_HAND ? "right" : "left", MyAvatar.sessionUUID];
            Entities.callEntityMethod(this.targetEntityID, "continueFarTrigger", args);
        };

        this.endFarTrigger = function (controllerData) {
            var args = [this.hand === RIGHT_HAND ? "right" : "left", MyAvatar.sessionUUID];
            Entities.callEntityMethod(this.targetEntityID, "stopFarTrigger", args);
        };

        this.isReady = function (controllerData) {
            this.targetEntityID = null;
            if (controllerData.triggerClicks[this.hand] === 0) {
                return makeRunningValues(false, [], []);
            }

            var targetProps = this.getTargetProps(controllerData);
            if (targetProps) {
                this.targetEntityID = targetProps.id;
                this.startFarTrigger(controllerData);
                return makeRunningValues(true, [this.targetEntityID], []);
            } else {
                return makeRunningValues(false, [], []);
            }
        };

        this.run = function (controllerData) {
            var targetEntity = controllerData.rayPicks[this.hand].objectID;
            if (controllerData.triggerClicks[this.hand] === 0 || this.targetEntityID !== targetEntity) {
                this.endFarTrigger(controllerData);
                return makeRunningValues(false, [], []);
            }
            this.continueFarTrigger(controllerData);
            return makeRunningValues(true, [this.targetEntityID], []);
        };
    }

    var leftFarTriggerEntity = new FarTriggerEntity(LEFT_HAND);
    var rightFarTriggerEntity = new FarTriggerEntity(RIGHT_HAND);

    enableDispatcherModule("LeftFarTriggerEntity", leftFarTriggerEntity);
    enableDispatcherModule("RightFarTriggerEntity", rightFarTriggerEntity);

    function cleanup() {
        disableDispatcherModule("LeftFarTriggerEntity");
        disableDispatcherModule("RightFarTriggerEntity");
    }
    Script.scriptEnding.connect(cleanup);
}());
