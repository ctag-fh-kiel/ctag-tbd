// ── Sortable.min.js ───
/*! Sortable 1.10.2 - MIT | git://github.com/SortableJS/Sortable.git */
!function(t,e){"object"==typeof exports&&"undefined"!=typeof module?module.exports=e():"function"==typeof define&&define.amd?define(e):(t=t||self).Sortable=e()}(this,function(){"use strict";function o(t){return(o="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function(t){return typeof t}:function(t){return t&&"function"==typeof Symbol&&t.constructor===Symbol&&t!==Symbol.prototype?"symbol":typeof t})(t)}function a(){return(a=Object.assign||function(t){for(var e=1;e<arguments.length;e++){var n=arguments[e];for(var o in n)Object.prototype.hasOwnProperty.call(n,o)&&(t[o]=n[o])}return t}).apply(this,arguments)}function I(i){for(var t=1;t<arguments.length;t++){var r=null!=arguments[t]?arguments[t]:{},e=Object.keys(r);"function"==typeof Object.getOwnPropertySymbols&&(e=e.concat(Object.getOwnPropertySymbols(r).filter(function(t){return Object.getOwnPropertyDescriptor(r,t).enumerable}))),e.forEach(function(t){var e,n,o;e=i,o=r[n=t],n in e?Object.defineProperty(e,n,{value:o,enumerable:!0,configurable:!0,writable:!0}):e[n]=o})}return i}function l(t,e){if(null==t)return{};var n,o,i=function(t,e){if(null==t)return{};var n,o,i={},r=Object.keys(t);for(o=0;o<r.length;o++)n=r[o],0<=e.indexOf(n)||(i[n]=t[n]);return i}(t,e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(t);for(o=0;o<r.length;o++)n=r[o],0<=e.indexOf(n)||Object.prototype.propertyIsEnumerable.call(t,n)&&(i[n]=t[n])}return i}function e(t){return function(t){if(Array.isArray(t)){for(var e=0,n=new Array(t.length);e<t.length;e++)n[e]=t[e];return n}}(t)||function(t){if(Symbol.iterator in Object(t)||"[object Arguments]"===Object.prototype.toString.call(t))return Array.from(t)}(t)||function(){throw new TypeError("Invalid attempt to spread non-iterable instance")}()}function t(t){if("undefined"!=typeof window&&window.navigator)return!!navigator.userAgent.match(t)}var w=t(/(?:Trident.*rv[ :]?11\.|msie|iemobile|Windows Phone)/i),E=t(/Edge/i),c=t(/firefox/i),s=t(/safari/i)&&!t(/chrome/i)&&!t(/android/i),n=t(/iP(ad|od|hone)/i),i=t(/chrome/i)&&t(/android/i),r={capture:!1,passive:!1};function u(t,e,n){t.addEventListener(e,n,!w&&r)}function d(t,e,n){t.removeEventListener(e,n,!w&&r)}function h(t,e){if(e){if(">"===e[0]&&(e=e.substring(1)),t)try{if(t.matches)return t.matches(e);if(t.msMatchesSelector)return t.msMatchesSelector(e);if(t.webkitMatchesSelector)return t.webkitMatchesSelector(e)}catch(t){return!1}return!1}}function P(t,e,n,o){if(t){n=n||document;do{if(null!=e&&(">"===e[0]?t.parentNode===n&&h(t,e):h(t,e))||o&&t===n)return t;if(t===n)break}while(t=(i=t).host&&i!==document&&i.host.nodeType?i.host:i.parentNode)}var i;return null}var f,p=/\s+/g;function k(t,e,n){if(t&&e)if(t.classList)t.classList[n?"add":"remove"](e);else{var o=(" "+t.className+" ").replace(p," ").replace(" "+e+" "," ");t.className=(o+(n?" "+e:"")).replace(p," ")}}function R(t,e,n){var o=t&&t.style;if(o){if(void 0===n)return document.defaultView&&document.defaultView.getComputedStyle?n=document.defaultView.getComputedStyle(t,""):t.currentStyle&&(n=t.currentStyle),void 0===e?n:n[e];e in o||-1!==e.indexOf("webkit")||(e="-webkit-"+e),o[e]=n+("string"==typeof n?"":"px")}}function v(t,e){var n="";if("string"==typeof t)n=t;else do{var o=R(t,"transform");o&&"none"!==o&&(n=o+" "+n)}while(!e&&(t=t.parentNode));var i=window.DOMMatrix||window.WebKitCSSMatrix||window.CSSMatrix||window.MSCSSMatrix;return i&&new i(n)}function g(t,e,n){if(t){var o=t.getElementsByTagName(e),i=0,r=o.length;if(n)for(;i<r;i++)n(o[i],i);return o}return[]}function N(){var t=document.scrollingElement;return t||document.documentElement}function X(t,e,n,o,i){if(t.getBoundingClientRect||t===window){var r,a,l,s,c,u,d;if(d=t!==window&&t!==N()?(a=(r=t.getBoundingClientRect()).top,l=r.left,s=r.bottom,c=r.right,u=r.height,r.width):(l=a=0,s=window.innerHeight,c=window.innerWidth,u=window.innerHeight,window.innerWidth),(e||n)&&t!==window&&(i=i||t.parentNode,!w))do{if(i&&i.getBoundingClientRect&&("none"!==R(i,"transform")||n&&"static"!==R(i,"position"))){var h=i.getBoundingClientRect();a-=h.top+parseInt(R(i,"border-top-width")),l-=h.left+parseInt(R(i,"border-left-width")),s=a+r.height,c=l+r.width;break}}while(i=i.parentNode);if(o&&t!==window){var f=v(i||t),p=f&&f.a,g=f&&f.d;f&&(s=(a/=g)+(u/=g),c=(l/=p)+(d/=p))}return{top:a,left:l,bottom:s,right:c,width:d,height:u}}}function Y(t,e,n){for(var o=H(t,!0),i=X(t)[e];o;){var r=X(o)[n];if(!("top"===n||"left"===n?r<=i:i<=r))return o;if(o===N())break;o=H(o,!1)}return!1}function m(t,e,n){for(var o=0,i=0,r=t.children;i<r.length;){if("none"!==r[i].style.display&&r[i]!==Rt.ghost&&r[i]!==Rt.dragged&&P(r[i],n.draggable,t,!1)){if(o===e)return r[i];o++}i++}return null}function B(t,e){for(var n=t.lastElementChild;n&&(n===Rt.ghost||"none"===R(n,"display")||e&&!h(n,e));)n=n.previousElementSibling;return n||null}function F(t,e){var n=0;if(!t||!t.parentNode)return-1;for(;t=t.previousElementSibling;)"TEMPLATE"===t.nodeName.toUpperCase()||t===Rt.clone||e&&!h(t,e)||n++;return n}function b(t){var e=0,n=0,o=N();if(t)do{var i=v(t),r=i.a,a=i.d;e+=t.scrollLeft*r,n+=t.scrollTop*a}while(t!==o&&(t=t.parentNode));return[e,n]}function H(t,e){if(!t||!t.getBoundingClientRect)return N();var n=t,o=!1;do{if(n.clientWidth<n.scrollWidth||n.clientHeight<n.scrollHeight){var i=R(n);if(n.clientWidth<n.scrollWidth&&("auto"==i.overflowX||"scroll"==i.overflowX)||n.clientHeight<n.scrollHeight&&("auto"==i.overflowY||"scroll"==i.overflowY)){if(!n.getBoundingClientRect||n===document.body)return N();if(o||e)return n;o=!0}}}while(n=n.parentNode);return N()}function y(t,e){return Math.round(t.top)===Math.round(e.top)&&Math.round(t.left)===Math.round(e.left)&&Math.round(t.height)===Math.round(e.height)&&Math.round(t.width)===Math.round(e.width)}function D(e,n){return function(){if(!f){var t=arguments;1===t.length?e.call(this,t[0]):e.apply(this,t),f=setTimeout(function(){f=void 0},n)}}}function L(t,e,n){t.scrollLeft+=e,t.scrollTop+=n}function S(t){var e=window.Polymer,n=window.jQuery||window.Zepto;return e&&e.dom?e.dom(t).cloneNode(!0):n?n(t).clone(!0)[0]:t.cloneNode(!0)}function _(t,e){R(t,"position","absolute"),R(t,"top",e.top),R(t,"left",e.left),R(t,"width",e.width),R(t,"height",e.height)}function C(t){R(t,"position",""),R(t,"top",""),R(t,"left",""),R(t,"width",""),R(t,"height","")}var j="Sortable"+(new Date).getTime();function T(){var e,o=[];return{captureAnimationState:function(){o=[],this.options.animation&&[].slice.call(this.el.children).forEach(function(t){if("none"!==R(t,"display")&&t!==Rt.ghost){o.push({target:t,rect:X(t)});var e=I({},o[o.length-1].rect);if(t.thisAnimationDuration){var n=v(t,!0);n&&(e.top-=n.f,e.left-=n.e)}t.fromRect=e}})},addAnimationState:function(t){o.push(t)},removeAnimationState:function(t){o.splice(function(t,e){for(var n in t)if(t.hasOwnProperty(n))for(var o in e)if(e.hasOwnProperty(o)&&e[o]===t[n][o])return Number(n);return-1}(o,{target:t}),1)},animateAll:function(t){var c=this;if(!this.options.animation)return clearTimeout(e),void("function"==typeof t&&t());var u=!1,d=0;o.forEach(function(t){var e=0,n=t.target,o=n.fromRect,i=X(n),r=n.prevFromRect,a=n.prevToRect,l=t.rect,s=v(n,!0);s&&(i.top-=s.f,i.left-=s.e),n.toRect=i,n.thisAnimationDuration&&y(r,i)&&!y(o,i)&&(l.top-i.top)/(l.left-i.left)==(o.top-i.top)/(o.left-i.left)&&(e=function(t,e,n,o){return Math.sqrt(Math.pow(e.top-t.top,2)+Math.pow(e.left-t.left,2))/Math.sqrt(Math.pow(e.top-n.top,2)+Math.pow(e.left-n.left,2))*o.animation}(l,r,a,c.options)),y(i,o)||(n.prevFromRect=o,n.prevToRect=i,e||(e=c.options.animation),c.animate(n,l,i,e)),e&&(u=!0,d=Math.max(d,e),clearTimeout(n.animationResetTimer),n.animationResetTimer=setTimeout(function(){n.animationTime=0,n.prevFromRect=null,n.fromRect=null,n.prevToRect=null,n.thisAnimationDuration=null},e),n.thisAnimationDuration=e)}),clearTimeout(e),u?e=setTimeout(function(){"function"==typeof t&&t()},d):"function"==typeof t&&t(),o=[]},animate:function(t,e,n,o){if(o){R(t,"transition",""),R(t,"transform","");var i=v(this.el),r=i&&i.a,a=i&&i.d,l=(e.left-n.left)/(r||1),s=(e.top-n.top)/(a||1);t.animatingX=!!l,t.animatingY=!!s,R(t,"transform","translate3d("+l+"px,"+s+"px,0)"),function(t){t.offsetWidth}(t),R(t,"transition","transform "+o+"ms"+(this.options.easing?" "+this.options.easing:"")),R(t,"transform","translate3d(0,0,0)"),"number"==typeof t.animated&&clearTimeout(t.animated),t.animated=setTimeout(function(){R(t,"transition",""),R(t,"transform",""),t.animated=!1,t.animatingX=!1,t.animatingY=!1},o)}}}}var x=[],M={initializeByDefault:!0},O={mount:function(t){for(var e in M)!M.hasOwnProperty(e)||e in t||(t[e]=M[e]);x.push(t)},pluginEvent:function(e,n,o){var t=this;this.eventCanceled=!1,o.cancel=function(){t.eventCanceled=!0};var i=e+"Global";x.forEach(function(t){n[t.pluginName]&&(n[t.pluginName][i]&&n[t.pluginName][i](I({sortable:n},o)),n.options[t.pluginName]&&n[t.pluginName][e]&&n[t.pluginName][e](I({sortable:n},o)))})},initializePlugins:function(o,i,r,t){for(var e in x.forEach(function(t){var e=t.pluginName;if(o.options[e]||t.initializeByDefault){var n=new t(o,i,o.options);n.sortable=o,n.options=o.options,o[e]=n,a(r,n.defaults)}}),o.options)if(o.options.hasOwnProperty(e)){var n=this.modifyOption(o,e,o.options[e]);void 0!==n&&(o.options[e]=n)}},getEventProperties:function(e,n){var o={};return x.forEach(function(t){"function"==typeof t.eventProperties&&a(o,t.eventProperties.call(n[t.pluginName],e))}),o},modifyOption:function(e,n,o){var i;return x.forEach(function(t){e[t.pluginName]&&t.optionListeners&&"function"==typeof t.optionListeners[n]&&(i=t.optionListeners[n].call(e[t.pluginName],o))}),i}};function A(t){var e=t.sortable,n=t.rootEl,o=t.name,i=t.targetEl,r=t.cloneEl,a=t.toEl,l=t.fromEl,s=t.oldIndex,c=t.newIndex,u=t.oldDraggableIndex,d=t.newDraggableIndex,h=t.originalEvent,f=t.putSortable,p=t.extraEventProperties;if(e=e||n&&n[j]){var g,v=e.options,m="on"+o.charAt(0).toUpperCase()+o.substr(1);!window.CustomEvent||w||E?(g=document.createEvent("Event")).initEvent(o,!0,!0):g=new CustomEvent(o,{bubbles:!0,cancelable:!0}),g.to=a||n,g.from=l||n,g.item=i||n,g.clone=r,g.oldIndex=s,g.newIndex=c,g.oldDraggableIndex=u,g.newDraggableIndex=d,g.originalEvent=h,g.pullMode=f?f.lastPutMode:void 0;var b=I({},p,O.getEventProperties(o,e));for(var y in b)g[y]=b[y];n&&n.dispatchEvent(g),v[m]&&v[m].call(e,g)}}function K(t,e,n){var o=2<arguments.length&&void 0!==n?n:{},i=o.evt,r=l(o,["evt"]);O.pluginEvent.bind(Rt)(t,e,I({dragEl:z,parentEl:G,ghostEl:U,rootEl:q,nextEl:V,lastDownEl:Z,cloneEl:Q,cloneHidden:$,dragStarted:dt,putSortable:it,activeSortable:Rt.active,originalEvent:i,oldIndex:J,oldDraggableIndex:et,newIndex:tt,newDraggableIndex:nt,hideGhostForTarget:Nt,unhideGhostForTarget:It,cloneNowHidden:function(){$=!0},cloneNowShown:function(){$=!1},dispatchSortableEvent:function(t){W({sortable:e,name:t,originalEvent:i})}},r))}function W(t){A(I({putSortable:it,cloneEl:Q,targetEl:z,rootEl:q,oldIndex:J,oldDraggableIndex:et,newIndex:tt,newDraggableIndex:nt},t))}var z,G,U,q,V,Z,Q,$,J,tt,et,nt,ot,it,rt,at,lt,st,ct,ut,dt,ht,ft,pt,gt,vt=!1,mt=!1,bt=[],yt=!1,wt=!1,Et=[],Dt=!1,St=[],_t="undefined"!=typeof document,Ct=n,Tt=E||w?"cssFloat":"float",xt=_t&&!i&&!n&&"draggable"in document.createElement("div"),Mt=function(){if(_t){if(w)return!1;var t=document.createElement("x");return t.style.cssText="pointer-events:auto","auto"===t.style.pointerEvents}}(),Ot=function(t,e){var n=R(t),o=parseInt(n.width)-parseInt(n.paddingLeft)-parseInt(n.paddingRight)-parseInt(n.borderLeftWidth)-parseInt(n.borderRightWidth),i=m(t,0,e),r=m(t,1,e),a=i&&R(i),l=r&&R(r),s=a&&parseInt(a.marginLeft)+parseInt(a.marginRight)+X(i).width,c=l&&parseInt(l.marginLeft)+parseInt(l.marginRight)+X(r).width;if("flex"===n.display)return"column"===n.flexDirection||"column-reverse"===n.flexDirection?"vertical":"horizontal";if("grid"===n.display)return n.gridTemplateColumns.split(" ").length<=1?"vertical":"horizontal";if(i&&a.float&&"none"!==a.float){var u="left"===a.float?"left":"right";return!r||"both"!==l.clear&&l.clear!==u?"horizontal":"vertical"}return i&&("block"===a.display||"flex"===a.display||"table"===a.display||"grid"===a.display||o<=s&&"none"===n[Tt]||r&&"none"===n[Tt]&&o<s+c)?"vertical":"horizontal"},At=function(t){function s(a,l){return function(t,e,n,o){var i=t.options.group.name&&e.options.group.name&&t.options.group.name===e.options.group.name;if(null==a&&(l||i))return!0;if(null==a||!1===a)return!1;if(l&&"clone"===a)return a;if("function"==typeof a)return s(a(t,e,n,o),l)(t,e,n,o);var r=(l?t:e).options.group.name;return!0===a||"string"==typeof a&&a===r||a.join&&-1<a.indexOf(r)}}var e={},n=t.group;n&&"object"==o(n)||(n={name:n}),e.name=n.name,e.checkPull=s(n.pull,!0),e.checkPut=s(n.put),e.revertClone=n.revertClone,t.group=e},Nt=function(){!Mt&&U&&R(U,"display","none")},It=function(){!Mt&&U&&R(U,"display","")};_t&&document.addEventListener("click",function(t){if(mt)return t.preventDefault(),t.stopPropagation&&t.stopPropagation(),t.stopImmediatePropagation&&t.stopImmediatePropagation(),mt=!1},!0);function Pt(t){if(z){var e=function(r,a){var l;return bt.some(function(t){if(!B(t)){var e=X(t),n=t[j].options.emptyInsertThreshold,o=r>=e.left-n&&r<=e.right+n,i=a>=e.top-n&&a<=e.bottom+n;return n&&o&&i?l=t:void 0}}),l}((t=t.touches?t.touches[0]:t).clientX,t.clientY);if(e){var n={};for(var o in t)t.hasOwnProperty(o)&&(n[o]=t[o]);n.target=n.rootEl=e,n.preventDefault=void 0,n.stopPropagation=void 0,e[j]._onDragOver(n)}}}function kt(t){z&&z.parentNode[j]._isOutsideThisEl(t.target)}function Rt(t,e){if(!t||!t.nodeType||1!==t.nodeType)throw"Sortable: `el` must be an HTMLElement, not ".concat({}.toString.call(t));this.el=t,this.options=e=a({},e),t[j]=this;var n={group:null,sort:!0,disabled:!1,store:null,handle:null,draggable:/^[uo]l$/i.test(t.nodeName)?">li":">*",swapThreshold:1,invertSwap:!1,invertedSwapThreshold:null,removeCloneOnHide:!0,direction:function(){return Ot(t,this.options)},ghostClass:"sortable-ghost",chosenClass:"sortable-chosen",dragClass:"sortable-drag",ignore:"a, img",filter:null,preventOnFilter:!0,animation:0,easing:null,setData:function(t,e){t.setData("Text",e.textContent)},dropBubble:!1,dragoverBubble:!1,dataIdAttr:"data-id",delay:0,delayOnTouchOnly:!1,touchStartThreshold:(Number.parseInt?Number:window).parseInt(window.devicePixelRatio,10)||1,forceFallback:!1,fallbackClass:"sortable-fallback",fallbackOnBody:!1,fallbackTolerance:0,fallbackOffset:{x:0,y:0},supportPointer:!1!==Rt.supportPointer&&"PointerEvent"in window,emptyInsertThreshold:5};for(var o in O.initializePlugins(this,t,n),n)o in e||(e[o]=n[o]);for(var i in At(e),this)"_"===i.charAt(0)&&"function"==typeof this[i]&&(this[i]=this[i].bind(this));this.nativeDraggable=!e.forceFallback&&xt,this.nativeDraggable&&(this.options.touchStartThreshold=1),e.supportPointer?u(t,"pointerdown",this._onTapStart):(u(t,"mousedown",this._onTapStart),u(t,"touchstart",this._onTapStart)),this.nativeDraggable&&(u(t,"dragover",this),u(t,"dragenter",this)),bt.push(this.el),e.store&&e.store.get&&this.sort(e.store.get(this)||[]),a(this,T())}function Xt(t,e,n,o,i,r,a,l){var s,c,u=t[j],d=u.options.onMove;return!window.CustomEvent||w||E?(s=document.createEvent("Event")).initEvent("move",!0,!0):s=new CustomEvent("move",{bubbles:!0,cancelable:!0}),s.to=e,s.from=t,s.dragged=n,s.draggedRect=o,s.related=i||e,s.relatedRect=r||X(e),s.willInsertAfter=l,s.originalEvent=a,t.dispatchEvent(s),d&&(c=d.call(u,s,a)),c}function Yt(t){t.draggable=!1}function Bt(){Dt=!1}function Ft(t){for(var e=t.tagName+t.className+t.src+t.href+t.textContent,n=e.length,o=0;n--;)o+=e.charCodeAt(n);return o.toString(36)}function Ht(t){return setTimeout(t,0)}function Lt(t){return clearTimeout(t)}Rt.prototype={constructor:Rt,_isOutsideThisEl:function(t){this.el.contains(t)||t===this.el||(ht=null)},_getDirection:function(t,e){return"function"==typeof this.options.direction?this.options.direction.call(this,t,e,z):this.options.direction},_onTapStart:function(e){if(e.cancelable){var n=this,o=this.el,t=this.options,i=t.preventOnFilter,r=e.type,a=e.touches&&e.touches[0]||e.pointerType&&"touch"===e.pointerType&&e,l=(a||e).target,s=e.target.shadowRoot&&(e.path&&e.path[0]||e.composedPath&&e.composedPath()[0])||l,c=t.filter;if(function(t){St.length=0;var e=t.getElementsByTagName("input"),n=e.length;for(;n--;){var o=e[n];o.checked&&St.push(o)}}(o),!z&&!(/mousedown|pointerdown/.test(r)&&0!==e.button||t.disabled||s.isContentEditable||(l=P(l,t.draggable,o,!1))&&l.animated||Z===l)){if(J=F(l),et=F(l,t.draggable),"function"==typeof c){if(c.call(this,e,l,this))return W({sortable:n,rootEl:s,name:"filter",targetEl:l,toEl:o,fromEl:o}),K("filter",n,{evt:e}),void(i&&e.cancelable&&e.preventDefault())}else if(c&&(c=c.split(",").some(function(t){if(t=P(s,t.trim(),o,!1))return W({sortable:n,rootEl:t,name:"filter",targetEl:l,fromEl:o,toEl:o}),K("filter",n,{evt:e}),!0})))return void(i&&e.cancelable&&e.preventDefault());t.handle&&!P(s,t.handle,o,!1)||this._prepareDragStart(e,a,l)}}},_prepareDragStart:function(t,e,n){var o,i=this,r=i.el,a=i.options,l=r.ownerDocument;if(n&&!z&&n.parentNode===r){var s=X(n);if(q=r,G=(z=n).parentNode,V=z.nextSibling,Z=n,ot=a.group,rt={target:Rt.dragged=z,clientX:(e||t).clientX,clientY:(e||t).clientY},ct=rt.clientX-s.left,ut=rt.clientY-s.top,this._lastX=(e||t).clientX,this._lastY=(e||t).clientY,z.style["will-change"]="all",o=function(){K("delayEnded",i,{evt:t}),Rt.eventCanceled?i._onDrop():(i._disableDelayedDragEvents(),!c&&i.nativeDraggable&&(z.draggable=!0),i._triggerDragStart(t,e),W({sortable:i,name:"choose",originalEvent:t}),k(z,a.chosenClass,!0))},a.ignore.split(",").forEach(function(t){g(z,t.trim(),Yt)}),u(l,"dragover",Pt),u(l,"mousemove",Pt),u(l,"touchmove",Pt),u(l,"mouseup",i._onDrop),u(l,"touchend",i._onDrop),u(l,"touchcancel",i._onDrop),c&&this.nativeDraggable&&(this.options.touchStartThreshold=4,z.draggable=!0),K("delayStart",this,{evt:t}),!a.delay||a.delayOnTouchOnly&&!e||this.nativeDraggable&&(E||w))o();else{if(Rt.eventCanceled)return void this._onDrop();u(l,"mouseup",i._disableDelayedDrag),u(l,"touchend",i._disableDelayedDrag),u(l,"touchcancel",i._disableDelayedDrag),u(l,"mousemove",i._delayedDragTouchMoveHandler),u(l,"touchmove",i._delayedDragTouchMoveHandler),a.supportPointer&&u(l,"pointermove",i._delayedDragTouchMoveHandler),i._dragStartTimer=setTimeout(o,a.delay)}}},_delayedDragTouchMoveHandler:function(t){var e=t.touches?t.touches[0]:t;Math.max(Math.abs(e.clientX-this._lastX),Math.abs(e.clientY-this._lastY))>=Math.floor(this.options.touchStartThreshold/(this.nativeDraggable&&window.devicePixelRatio||1))&&this._disableDelayedDrag()},_disableDelayedDrag:function(){z&&Yt(z),clearTimeout(this._dragStartTimer),this._disableDelayedDragEvents()},_disableDelayedDragEvents:function(){var t=this.el.ownerDocument;d(t,"mouseup",this._disableDelayedDrag),d(t,"touchend",this._disableDelayedDrag),d(t,"touchcancel",this._disableDelayedDrag),d(t,"mousemove",this._delayedDragTouchMoveHandler),d(t,"touchmove",this._delayedDragTouchMoveHandler),d(t,"pointermove",this._delayedDragTouchMoveHandler)},_triggerDragStart:function(t,e){e=e||"touch"==t.pointerType&&t,!this.nativeDraggable||e?this.options.supportPointer?u(document,"pointermove",this._onTouchMove):u(document,e?"touchmove":"mousemove",this._onTouchMove):(u(z,"dragend",this),u(q,"dragstart",this._onDragStart));try{document.selection?Ht(function(){document.selection.empty()}):window.getSelection().removeAllRanges()}catch(t){}},_dragStarted:function(t,e){if(vt=!1,q&&z){K("dragStarted",this,{evt:e}),this.nativeDraggable&&u(document,"dragover",kt);var n=this.options;t||k(z,n.dragClass,!1),k(z,n.ghostClass,!0),Rt.active=this,t&&this._appendGhost(),W({sortable:this,name:"start",originalEvent:e})}else this._nulling()},_emulateDragOver:function(){if(at){this._lastX=at.clientX,this._lastY=at.clientY,Nt();for(var t=document.elementFromPoint(at.clientX,at.clientY),e=t;t&&t.shadowRoot&&(t=t.shadowRoot.elementFromPoint(at.clientX,at.clientY))!==e;)e=t;if(z.parentNode[j]._isOutsideThisEl(t),e)do{if(e[j]){if(e[j]._onDragOver({clientX:at.clientX,clientY:at.clientY,target:t,rootEl:e})&&!this.options.dragoverBubble)break}t=e}while(e=e.parentNode);It()}},_onTouchMove:function(t){if(rt){var e=this.options,n=e.fallbackTolerance,o=e.fallbackOffset,i=t.touches?t.touches[0]:t,r=U&&v(U,!0),a=U&&r&&r.a,l=U&&r&&r.d,s=Ct&&gt&&b(gt),c=(i.clientX-rt.clientX+o.x)/(a||1)+(s?s[0]-Et[0]:0)/(a||1),u=(i.clientY-rt.clientY+o.y)/(l||1)+(s?s[1]-Et[1]:0)/(l||1);if(!Rt.active&&!vt){if(n&&Math.max(Math.abs(i.clientX-this._lastX),Math.abs(i.clientY-this._lastY))<n)return;this._onDragStart(t,!0)}if(U){r?(r.e+=c-(lt||0),r.f+=u-(st||0)):r={a:1,b:0,c:0,d:1,e:c,f:u};var d="matrix(".concat(r.a,",").concat(r.b,",").concat(r.c,",").concat(r.d,",").concat(r.e,",").concat(r.f,")");R(U,"webkitTransform",d),R(U,"mozTransform",d),R(U,"msTransform",d),R(U,"transform",d),lt=c,st=u,at=i}t.cancelable&&t.preventDefault()}},_appendGhost:function(){if(!U){var t=this.options.fallbackOnBody?document.body:q,e=X(z,!0,Ct,!0,t),n=this.options;if(Ct){for(gt=t;"static"===R(gt,"position")&&"none"===R(gt,"transform")&&gt!==document;)gt=gt.parentNode;gt!==document.body&&gt!==document.documentElement?(gt===document&&(gt=N()),e.top+=gt.scrollTop,e.left+=gt.scrollLeft):gt=N(),Et=b(gt)}k(U=z.cloneNode(!0),n.ghostClass,!1),k(U,n.fallbackClass,!0),k(U,n.dragClass,!0),R(U,"transition",""),R(U,"transform",""),R(U,"box-sizing","border-box"),R(U,"margin",0),R(U,"top",e.top),R(U,"left",e.left),R(U,"width",e.width),R(U,"height",e.height),R(U,"opacity","0.8"),R(U,"position",Ct?"absolute":"fixed"),R(U,"zIndex","100000"),R(U,"pointerEvents","none"),Rt.ghost=U,t.appendChild(U),R(U,"transform-origin",ct/parseInt(U.style.width)*100+"% "+ut/parseInt(U.style.height)*100+"%")}},_onDragStart:function(t,e){var n=this,o=t.dataTransfer,i=n.options;K("dragStart",this,{evt:t}),Rt.eventCanceled?this._onDrop():(K("setupClone",this),Rt.eventCanceled||((Q=S(z)).draggable=!1,Q.style["will-change"]="",this._hideClone(),k(Q,this.options.chosenClass,!1),Rt.clone=Q),n.cloneId=Ht(function(){K("clone",n),Rt.eventCanceled||(n.options.removeCloneOnHide||q.insertBefore(Q,z),n._hideClone(),W({sortable:n,name:"clone"}))}),e||k(z,i.dragClass,!0),e?(mt=!0,n._loopId=setInterval(n._emulateDragOver,50)):(d(document,"mouseup",n._onDrop),d(document,"touchend",n._onDrop),d(document,"touchcancel",n._onDrop),o&&(o.effectAllowed="move",i.setData&&i.setData.call(n,o,z)),u(document,"drop",n),R(z,"transform","translateZ(0)")),vt=!0,n._dragStartId=Ht(n._dragStarted.bind(n,e,t)),u(document,"selectstart",n),dt=!0,s&&R(document.body,"user-select","none"))},_onDragOver:function(n){var o,i,r,a,l=this.el,s=n.target,e=this.options,t=e.group,c=Rt.active,u=ot===t,d=e.sort,h=it||c,f=this,p=!1;if(!Dt){if(void 0!==n.preventDefault&&n.cancelable&&n.preventDefault(),s=P(s,e.draggable,l,!0),M("dragOver"),Rt.eventCanceled)return p;if(z.contains(n.target)||s.animated&&s.animatingX&&s.animatingY||f._ignoreWhileAnimating===s)return A(!1);if(mt=!1,c&&!e.disabled&&(u?d||(r=!q.contains(z)):it===this||(this.lastPutMode=ot.checkPull(this,c,z,n))&&t.checkPut(this,c,z,n))){if(a="vertical"===this._getDirection(n,s),o=X(z),M("dragOverValid"),Rt.eventCanceled)return p;if(r)return G=q,O(),this._hideClone(),M("revert"),Rt.eventCanceled||(V?q.insertBefore(z,V):q.appendChild(z)),A(!0);var g=B(l,e.draggable);if(!g||function(t,e,n){var o=X(B(n.el,n.options.draggable));return e?t.clientX>o.right+10||t.clientX<=o.right&&t.clientY>o.bottom&&t.clientX>=o.left:t.clientX>o.right&&t.clientY>o.top||t.clientX<=o.right&&t.clientY>o.bottom+10}(n,a,this)&&!g.animated){if(g===z)return A(!1);if(g&&l===n.target&&(s=g),s&&(i=X(s)),!1!==Xt(q,l,z,o,s,i,n,!!s))return O(),l.appendChild(z),G=l,N(),A(!0)}else if(s.parentNode===l){i=X(s);var v,m,b,y=z.parentNode!==l,w=!function(t,e,n){var o=n?t.left:t.top,i=n?t.right:t.bottom,r=n?t.width:t.height,a=n?e.left:e.top,l=n?e.right:e.bottom,s=n?e.width:e.height;return o===a||i===l||o+r/2===a+s/2}(z.animated&&z.toRect||o,s.animated&&s.toRect||i,a),E=a?"top":"left",D=Y(s,"top","top")||Y(z,"top","top"),S=D?D.scrollTop:void 0;if(ht!==s&&(m=i[E],yt=!1,wt=!w&&e.invertSwap||y),0!==(v=function(t,e,n,o,i,r,a,l){var s=o?t.clientY:t.clientX,c=o?n.height:n.width,u=o?n.top:n.left,d=o?n.bottom:n.right,h=!1;if(!a)if(l&&pt<c*i){if(!yt&&(1===ft?u+c*r/2<s:s<d-c*r/2)&&(yt=!0),yt)h=!0;else if(1===ft?s<u+pt:d-pt<s)return-ft}else if(u+c*(1-i)/2<s&&s<d-c*(1-i)/2)return function(t){return F(z)<F(t)?1:-1}(e);if((h=h||a)&&(s<u+c*r/2||d-c*r/2<s))return u+c/2<s?1:-1;return 0}(n,s,i,a,w?1:e.swapThreshold,null==e.invertedSwapThreshold?e.swapThreshold:e.invertedSwapThreshold,wt,ht===s)))for(var _=F(z);_-=v,(b=G.children[_])&&("none"===R(b,"display")||b===U););if(0===v||b===s)return A(!1);ft=v;var C=(ht=s).nextElementSibling,T=!1,x=Xt(q,l,z,o,s,i,n,T=1===v);if(!1!==x)return 1!==x&&-1!==x||(T=1===x),Dt=!0,setTimeout(Bt,30),O(),T&&!C?l.appendChild(z):s.parentNode.insertBefore(z,T?C:s),D&&L(D,0,S-D.scrollTop),G=z.parentNode,void 0===m||wt||(pt=Math.abs(m-X(s)[E])),N(),A(!0)}if(l.contains(z))return A(!1)}return!1}function M(t,e){K(t,f,I({evt:n,isOwner:u,axis:a?"vertical":"horizontal",revert:r,dragRect:o,targetRect:i,canSort:d,fromSortable:h,target:s,completed:A,onMove:function(t,e){return Xt(q,l,z,o,t,X(t),n,e)},changed:N},e))}function O(){M("dragOverAnimationCapture"),f.captureAnimationState(),f!==h&&h.captureAnimationState()}function A(t){return M("dragOverCompleted",{insertion:t}),t&&(u?c._hideClone():c._showClone(f),f!==h&&(k(z,it?it.options.ghostClass:c.options.ghostClass,!1),k(z,e.ghostClass,!0)),it!==f&&f!==Rt.active?it=f:f===Rt.active&&it&&(it=null),h===f&&(f._ignoreWhileAnimating=s),f.animateAll(function(){M("dragOverAnimationComplete"),f._ignoreWhileAnimating=null}),f!==h&&(h.animateAll(),h._ignoreWhileAnimating=null)),(s===z&&!z.animated||s===l&&!s.animated)&&(ht=null),e.dragoverBubble||n.rootEl||s===document||(z.parentNode[j]._isOutsideThisEl(n.target),t||Pt(n)),!e.dragoverBubble&&n.stopPropagation&&n.stopPropagation(),p=!0}function N(){tt=F(z),nt=F(z,e.draggable),W({sortable:f,name:"change",toEl:l,newIndex:tt,newDraggableIndex:nt,originalEvent:n})}},_ignoreWhileAnimating:null,_offMoveEvents:function(){d(document,"mousemove",this._onTouchMove),d(document,"touchmove",this._onTouchMove),d(document,"pointermove",this._onTouchMove),d(document,"dragover",Pt),d(document,"mousemove",Pt),d(document,"touchmove",Pt)},_offUpEvents:function(){var t=this.el.ownerDocument;d(t,"mouseup",this._onDrop),d(t,"touchend",this._onDrop),d(t,"pointerup",this._onDrop),d(t,"touchcancel",this._onDrop),d(document,"selectstart",this)},_onDrop:function(t){var e=this.el,n=this.options;tt=F(z),nt=F(z,n.draggable),K("drop",this,{evt:t}),G=z&&z.parentNode,tt=F(z),nt=F(z,n.draggable),Rt.eventCanceled||(yt=wt=vt=!1,clearInterval(this._loopId),clearTimeout(this._dragStartTimer),Lt(this.cloneId),Lt(this._dragStartId),this.nativeDraggable&&(d(document,"drop",this),d(e,"dragstart",this._onDragStart)),this._offMoveEvents(),this._offUpEvents(),s&&R(document.body,"user-select",""),R(z,"transform",""),t&&(dt&&(t.cancelable&&t.preventDefault(),n.dropBubble||t.stopPropagation()),U&&U.parentNode&&U.parentNode.removeChild(U),(q===G||it&&"clone"!==it.lastPutMode)&&Q&&Q.parentNode&&Q.parentNode.removeChild(Q),z&&(this.nativeDraggable&&d(z,"dragend",this),Yt(z),z.style["will-change"]="",dt&&!vt&&k(z,it?it.options.ghostClass:this.options.ghostClass,!1),k(z,this.options.chosenClass,!1),W({sortable:this,name:"unchoose",toEl:G,newIndex:null,newDraggableIndex:null,originalEvent:t}),q!==G?(0<=tt&&(W({rootEl:G,name:"add",toEl:G,fromEl:q,originalEvent:t}),W({sortable:this,name:"remove",toEl:G,originalEvent:t}),W({rootEl:G,name:"sort",toEl:G,fromEl:q,originalEvent:t}),W({sortable:this,name:"sort",toEl:G,originalEvent:t})),it&&it.save()):tt!==J&&0<=tt&&(W({sortable:this,name:"update",toEl:G,originalEvent:t}),W({sortable:this,name:"sort",toEl:G,originalEvent:t})),Rt.active&&(null!=tt&&-1!==tt||(tt=J,nt=et),W({sortable:this,name:"end",toEl:G,originalEvent:t}),this.save())))),this._nulling()},_nulling:function(){K("nulling",this),q=z=G=U=V=Q=Z=$=rt=at=dt=tt=nt=J=et=ht=ft=it=ot=Rt.dragged=Rt.ghost=Rt.clone=Rt.active=null,St.forEach(function(t){t.checked=!0}),St.length=lt=st=0},handleEvent:function(t){switch(t.type){case"drop":case"dragend":this._onDrop(t);break;case"dragenter":case"dragover":z&&(this._onDragOver(t),function(t){t.dataTransfer&&(t.dataTransfer.dropEffect="move");t.cancelable&&t.preventDefault()}(t));break;case"selectstart":t.preventDefault()}},toArray:function(){for(var t,e=[],n=this.el.children,o=0,i=n.length,r=this.options;o<i;o++)P(t=n[o],r.draggable,this.el,!1)&&e.push(t.getAttribute(r.dataIdAttr)||Ft(t));return e},sort:function(t){var o={},i=this.el;this.toArray().forEach(function(t,e){var n=i.children[e];P(n,this.options.draggable,i,!1)&&(o[t]=n)},this),t.forEach(function(t){o[t]&&(i.removeChild(o[t]),i.appendChild(o[t]))})},save:function(){var t=this.options.store;t&&t.set&&t.set(this)},closest:function(t,e){return P(t,e||this.options.draggable,this.el,!1)},option:function(t,e){var n=this.options;if(void 0===e)return n[t];var o=O.modifyOption(this,t,e);n[t]=void 0!==o?o:e,"group"===t&&At(n)},destroy:function(){K("destroy",this);var t=this.el;t[j]=null,d(t,"mousedown",this._onTapStart),d(t,"touchstart",this._onTapStart),d(t,"pointerdown",this._onTapStart),this.nativeDraggable&&(d(t,"dragover",this),d(t,"dragenter",this)),Array.prototype.forEach.call(t.querySelectorAll("[draggable]"),function(t){t.removeAttribute("draggable")}),this._onDrop(),this._disableDelayedDragEvents(),bt.splice(bt.indexOf(this.el),1),this.el=t=null},_hideClone:function(){if(!$){if(K("hideClone",this),Rt.eventCanceled)return;R(Q,"display","none"),this.options.removeCloneOnHide&&Q.parentNode&&Q.parentNode.removeChild(Q),$=!0}},_showClone:function(t){if("clone"===t.lastPutMode){if($){if(K("showClone",this),Rt.eventCanceled)return;q.contains(z)&&!this.options.group.revertClone?q.insertBefore(Q,z):V?q.insertBefore(Q,V):q.appendChild(Q),this.options.group.revertClone&&this.animate(z,Q),R(Q,"display",""),$=!1}}else this._hideClone()}},_t&&u(document,"touchmove",function(t){(Rt.active||vt)&&t.cancelable&&t.preventDefault()}),Rt.utils={on:u,off:d,css:R,find:g,is:function(t,e){return!!P(t,e,t,!1)},extend:function(t,e){if(t&&e)for(var n in e)e.hasOwnProperty(n)&&(t[n]=e[n]);return t},throttle:D,closest:P,toggleClass:k,clone:S,index:F,nextTick:Ht,cancelNextTick:Lt,detectDirection:Ot,getChild:m},Rt.get=function(t){return t[j]},Rt.mount=function(){for(var t=arguments.length,e=new Array(t),n=0;n<t;n++)e[n]=arguments[n];e[0].constructor===Array&&(e=e[0]),e.forEach(function(t){if(!t.prototype||!t.prototype.constructor)throw"Sortable: Mounted plugin must be a constructor function, not ".concat({}.toString.call(t));t.utils&&(Rt.utils=I({},Rt.utils,t.utils)),O.mount(t)})},Rt.create=function(t,e){return new Rt(t,e)};var jt,Kt,Wt,zt,Gt,Ut,qt=[],Vt=!(Rt.version="1.10.2");function Zt(){qt.forEach(function(t){clearInterval(t.pid)}),qt=[]}function Qt(){clearInterval(Ut)}function $t(t){var e=t.originalEvent,n=t.putSortable,o=t.dragEl,i=t.activeSortable,r=t.dispatchSortableEvent,a=t.hideGhostForTarget,l=t.unhideGhostForTarget;if(e){var s=n||i;a();var c=e.changedTouches&&e.changedTouches.length?e.changedTouches[0]:e,u=document.elementFromPoint(c.clientX,c.clientY);l(),s&&!s.el.contains(u)&&(r("spill"),this.onSpill({dragEl:o,putSortable:n}))}}var Jt,te=D(function(n,t,e,o){if(t.scroll){var i,r=(n.touches?n.touches[0]:n).clientX,a=(n.touches?n.touches[0]:n).clientY,l=t.scrollSensitivity,s=t.scrollSpeed,c=N(),u=!1;Kt!==e&&(Kt=e,Zt(),jt=t.scroll,i=t.scrollFn,!0===jt&&(jt=H(e,!0)));var d=0,h=jt;do{var f=h,p=X(f),g=p.top,v=p.bottom,m=p.left,b=p.right,y=p.width,w=p.height,E=void 0,D=void 0,S=f.scrollWidth,_=f.scrollHeight,C=R(f),T=f.scrollLeft,x=f.scrollTop;D=f===c?(E=y<S&&("auto"===C.overflowX||"scroll"===C.overflowX||"visible"===C.overflowX),w<_&&("auto"===C.overflowY||"scroll"===C.overflowY||"visible"===C.overflowY)):(E=y<S&&("auto"===C.overflowX||"scroll"===C.overflowX),w<_&&("auto"===C.overflowY||"scroll"===C.overflowY));var M=E&&(Math.abs(b-r)<=l&&T+y<S)-(Math.abs(m-r)<=l&&!!T),O=D&&(Math.abs(v-a)<=l&&x+w<_)-(Math.abs(g-a)<=l&&!!x);if(!qt[d])for(var A=0;A<=d;A++)qt[A]||(qt[A]={});qt[d].vx==M&&qt[d].vy==O&&qt[d].el===f||(qt[d].el=f,qt[d].vx=M,qt[d].vy=O,clearInterval(qt[d].pid),0==M&&0==O||(u=!0,qt[d].pid=setInterval(function(){o&&0===this.layer&&Rt.active._onTouchMove(Gt);var t=qt[this.layer].vy?qt[this.layer].vy*s:0,e=qt[this.layer].vx?qt[this.layer].vx*s:0;"function"==typeof i&&"continue"!==i.call(Rt.dragged.parentNode[j],e,t,n,Gt,qt[this.layer].el)||L(qt[this.layer].el,e,t)}.bind({layer:d}),24))),d++}while(t.bubbleScroll&&h!==c&&(h=H(h,!1)));Vt=u}},30);function ee(){}function ne(){}ee.prototype={startIndex:null,dragStart:function(t){var e=t.oldDraggableIndex;this.startIndex=e},onSpill:function(t){var e=t.dragEl,n=t.putSortable;this.sortable.captureAnimationState(),n&&n.captureAnimationState();var o=m(this.sortable.el,this.startIndex,this.options);o?this.sortable.el.insertBefore(e,o):this.sortable.el.appendChild(e),this.sortable.animateAll(),n&&n.animateAll()},drop:$t},a(ee,{pluginName:"revertOnSpill"}),ne.prototype={onSpill:function(t){var e=t.dragEl,n=t.putSortable||this.sortable;n.captureAnimationState(),e.parentNode&&e.parentNode.removeChild(e),n.animateAll()},drop:$t},a(ne,{pluginName:"removeOnSpill"});var oe,ie,re,ae,le,se=[],ce=[],ue=!1,de=!1,he=!1;function fe(o,i){ce.forEach(function(t,e){var n=i.children[t.sortableIndex+(o?Number(e):0)];n?i.insertBefore(t,n):i.appendChild(t)})}function pe(){se.forEach(function(t){t!==re&&t.parentNode&&t.parentNode.removeChild(t)})}return Rt.mount(new function(){function t(){for(var t in this.defaults={scroll:!0,scrollSensitivity:30,scrollSpeed:10,bubbleScroll:!0},this)"_"===t.charAt(0)&&"function"==typeof this[t]&&(this[t]=this[t].bind(this))}return t.prototype={dragStarted:function(t){var e=t.originalEvent;this.sortable.nativeDraggable?u(document,"dragover",this._handleAutoScroll):this.options.supportPointer?u(document,"pointermove",this._handleFallbackAutoScroll):e.touches?u(document,"touchmove",this._handleFallbackAutoScroll):u(document,"mousemove",this._handleFallbackAutoScroll)},dragOverCompleted:function(t){var e=t.originalEvent;this.options.dragOverBubble||e.rootEl||this._handleAutoScroll(e)},drop:function(){this.sortable.nativeDraggable?d(document,"dragover",this._handleAutoScroll):(d(document,"pointermove",this._handleFallbackAutoScroll),d(document,"touchmove",this._handleFallbackAutoScroll),d(document,"mousemove",this._handleFallbackAutoScroll)),Qt(),Zt(),clearTimeout(f),f=void 0},nulling:function(){Gt=Kt=jt=Vt=Ut=Wt=zt=null,qt.length=0},_handleFallbackAutoScroll:function(t){this._handleAutoScroll(t,!0)},_handleAutoScroll:function(e,n){var o=this,i=(e.touches?e.touches[0]:e).clientX,r=(e.touches?e.touches[0]:e).clientY,t=document.elementFromPoint(i,r);if(Gt=e,n||E||w||s){te(e,this.options,t,n);var a=H(t,!0);!Vt||Ut&&i===Wt&&r===zt||(Ut&&Qt(),Ut=setInterval(function(){var t=H(document.elementFromPoint(i,r),!0);t!==a&&(a=t,Zt()),te(e,o.options,t,n)},10),Wt=i,zt=r)}else{if(!this.options.bubbleScroll||H(t,!0)===N())return void Zt();te(e,this.options,H(t,!1),!1)}}},a(t,{pluginName:"scroll",initializeByDefault:!0})}),Rt.mount(ne,ee),Rt.mount(new function(){function t(){this.defaults={swapClass:"sortable-swap-highlight"}}return t.prototype={dragStart:function(t){var e=t.dragEl;Jt=e},dragOverValid:function(t){var e=t.completed,n=t.target,o=t.onMove,i=t.activeSortable,r=t.changed,a=t.cancel;if(i.options.swap){var l=this.sortable.el,s=this.options;if(n&&n!==l){var c=Jt;Jt=!1!==o(n)?(k(n,s.swapClass,!0),n):null,c&&c!==Jt&&k(c,s.swapClass,!1)}r(),e(!0),a()}},drop:function(t){var e=t.activeSortable,n=t.putSortable,o=t.dragEl,i=n||this.sortable,r=this.options;Jt&&k(Jt,r.swapClass,!1),Jt&&(r.swap||n&&n.options.swap)&&o!==Jt&&(i.captureAnimationState(),i!==e&&e.captureAnimationState(),function(t,e){var n,o,i=t.parentNode,r=e.parentNode;if(!i||!r||i.isEqualNode(e)||r.isEqualNode(t))return;n=F(t),o=F(e),i.isEqualNode(r)&&n<o&&o++;i.insertBefore(e,i.children[n]),r.insertBefore(t,r.children[o])}(o,Jt),i.animateAll(),i!==e&&e.animateAll())},nulling:function(){Jt=null}},a(t,{pluginName:"swap",eventProperties:function(){return{swapItem:Jt}}})}),Rt.mount(new function(){function t(o){for(var t in this)"_"===t.charAt(0)&&"function"==typeof this[t]&&(this[t]=this[t].bind(this));o.options.supportPointer?u(document,"pointerup",this._deselectMultiDrag):(u(document,"mouseup",this._deselectMultiDrag),u(document,"touchend",this._deselectMultiDrag)),u(document,"keydown",this._checkKeyDown),u(document,"keyup",this._checkKeyUp),this.defaults={selectedClass:"sortable-selected",multiDragKey:null,setData:function(t,e){var n="";se.length&&ie===o?se.forEach(function(t,e){n+=(e?", ":"")+t.textContent}):n=e.textContent,t.setData("Text",n)}}}return t.prototype={multiDragKeyDown:!1,isMultiDrag:!1,delayStartGlobal:function(t){var e=t.dragEl;re=e},delayEnded:function(){this.isMultiDrag=~se.indexOf(re)},setupClone:function(t){var e=t.sortable,n=t.cancel;if(this.isMultiDrag){for(var o=0;o<se.length;o++)ce.push(S(se[o])),ce[o].sortableIndex=se[o].sortableIndex,ce[o].draggable=!1,ce[o].style["will-change"]="",k(ce[o],this.options.selectedClass,!1),se[o]===re&&k(ce[o],this.options.chosenClass,!1);e._hideClone(),n()}},clone:function(t){var e=t.sortable,n=t.rootEl,o=t.dispatchSortableEvent,i=t.cancel;this.isMultiDrag&&(this.options.removeCloneOnHide||se.length&&ie===e&&(fe(!0,n),o("clone"),i()))},showClone:function(t){var e=t.cloneNowShown,n=t.rootEl,o=t.cancel;this.isMultiDrag&&(fe(!1,n),ce.forEach(function(t){R(t,"display","")}),e(),le=!1,o())},hideClone:function(t){var e=this,n=(t.sortable,t.cloneNowHidden),o=t.cancel;this.isMultiDrag&&(ce.forEach(function(t){R(t,"display","none"),e.options.removeCloneOnHide&&t.parentNode&&t.parentNode.removeChild(t)}),n(),le=!0,o())},dragStartGlobal:function(t){t.sortable;!this.isMultiDrag&&ie&&ie.multiDrag._deselectMultiDrag(),se.forEach(function(t){t.sortableIndex=F(t)}),se=se.sort(function(t,e){return t.sortableIndex-e.sortableIndex}),he=!0},dragStarted:function(t){var e=this,n=t.sortable;if(this.isMultiDrag){if(this.options.sort&&(n.captureAnimationState(),this.options.animation)){se.forEach(function(t){t!==re&&R(t,"position","absolute")});var o=X(re,!1,!0,!0);se.forEach(function(t){t!==re&&_(t,o)}),ue=de=!0}n.animateAll(function(){ue=de=!1,e.options.animation&&se.forEach(function(t){C(t)}),e.options.sort&&pe()})}},dragOver:function(t){var e=t.target,n=t.completed,o=t.cancel;de&&~se.indexOf(e)&&(n(!1),o())},revert:function(t){var e=t.fromSortable,n=t.rootEl,o=t.sortable,i=t.dragRect;1<se.length&&(se.forEach(function(t){o.addAnimationState({target:t,rect:de?X(t):i}),C(t),t.fromRect=i,e.removeAnimationState(t)}),de=!1,function(o,i){se.forEach(function(t,e){var n=i.children[t.sortableIndex+(o?Number(e):0)];n?i.insertBefore(t,n):i.appendChild(t)})}(!this.options.removeCloneOnHide,n))},dragOverCompleted:function(t){var e=t.sortable,n=t.isOwner,o=t.insertion,i=t.activeSortable,r=t.parentEl,a=t.putSortable,l=this.options;if(o){if(n&&i._hideClone(),ue=!1,l.animation&&1<se.length&&(de||!n&&!i.options.sort&&!a)){var s=X(re,!1,!0,!0);se.forEach(function(t){t!==re&&(_(t,s),r.appendChild(t))}),de=!0}if(!n)if(de||pe(),1<se.length){var c=le;i._showClone(e),i.options.animation&&!le&&c&&ce.forEach(function(t){i.addAnimationState({target:t,rect:ae}),t.fromRect=ae,t.thisAnimationDuration=null})}else i._showClone(e)}},dragOverAnimationCapture:function(t){var e=t.dragRect,n=t.isOwner,o=t.activeSortable;if(se.forEach(function(t){t.thisAnimationDuration=null}),o.options.animation&&!n&&o.multiDrag.isMultiDrag){ae=a({},e);var i=v(re,!0);ae.top-=i.f,ae.left-=i.e}},dragOverAnimationComplete:function(){de&&(de=!1,pe())},drop:function(t){var e=t.originalEvent,n=t.rootEl,o=t.parentEl,i=t.sortable,r=t.dispatchSortableEvent,a=t.oldIndex,l=t.putSortable,s=l||this.sortable;if(e){var c=this.options,u=o.children;if(!he)if(c.multiDragKey&&!this.multiDragKeyDown&&this._deselectMultiDrag(),k(re,c.selectedClass,!~se.indexOf(re)),~se.indexOf(re))se.splice(se.indexOf(re),1),oe=null,A({sortable:i,rootEl:n,name:"deselect",targetEl:re,originalEvt:e});else{if(se.push(re),A({sortable:i,rootEl:n,name:"select",targetEl:re,originalEvt:e}),e.shiftKey&&oe&&i.el.contains(oe)){var d,h,f=F(oe),p=F(re);if(~f&&~p&&f!==p)for(d=f<p?(h=f,p):(h=p,f+1);h<d;h++)~se.indexOf(u[h])||(k(u[h],c.selectedClass,!0),se.push(u[h]),A({sortable:i,rootEl:n,name:"select",targetEl:u[h],originalEvt:e}))}else oe=re;ie=s}if(he&&this.isMultiDrag){if((o[j].options.sort||o!==n)&&1<se.length){var g=X(re),v=F(re,":not(."+this.options.selectedClass+")");if(!ue&&c.animation&&(re.thisAnimationDuration=null),s.captureAnimationState(),!ue&&(c.animation&&(re.fromRect=g,se.forEach(function(t){if(t.thisAnimationDuration=null,t!==re){var e=de?X(t):g;t.fromRect=e,s.addAnimationState({target:t,rect:e})}})),pe(),se.forEach(function(t){u[v]?o.insertBefore(t,u[v]):o.appendChild(t),v++}),a===F(re))){var m=!1;se.forEach(function(t){t.sortableIndex===F(t)||(m=!0)}),m&&r("update")}se.forEach(function(t){C(t)}),s.animateAll()}ie=s}(n===o||l&&"clone"!==l.lastPutMode)&&ce.forEach(function(t){t.parentNode&&t.parentNode.removeChild(t)})}},nullingGlobal:function(){this.isMultiDrag=he=!1,ce.length=0},destroyGlobal:function(){this._deselectMultiDrag(),d(document,"pointerup",this._deselectMultiDrag),d(document,"mouseup",this._deselectMultiDrag),d(document,"touchend",this._deselectMultiDrag),d(document,"keydown",this._checkKeyDown),d(document,"keyup",this._checkKeyUp)},_deselectMultiDrag:function(t){if(!(void 0!==he&&he||ie!==this.sortable||t&&P(t.target,this.options.draggable,this.sortable.el,!1)||t&&0!==t.button))for(;se.length;){var e=se[0];k(e,this.options.selectedClass,!1),se.shift(),A({sortable:this.sortable,rootEl:this.sortable.el,name:"deselect",targetEl:e,originalEvt:t})}},_checkKeyDown:function(t){t.key===this.options.multiDragKey&&(this.multiDragKeyDown=!0)},_checkKeyUp:function(t){t.key===this.options.multiDragKey&&(this.multiDragKeyDown=!1)}},a(t,{pluginName:"multiDrag",utils:{select:function(t){var e=t.parentNode[j];e&&e.options.multiDrag&&!~se.indexOf(t)&&(ie&&ie!==e&&(ie.multiDrag._deselectMultiDrag(),ie=e),k(t,e.options.selectedClass,!0),se.push(t))},deselect:function(t){var e=t.parentNode[j],n=se.indexOf(t);e&&e.options.multiDrag&&~n&&(k(t,e.options.selectedClass,!1),se.splice(n,1))}},eventProperties:function(){var n=this,o=[],i=[];return se.forEach(function(t){var e;o.push({multiDragElement:t,index:t.sortableIndex}),e=de&&t!==re?-1:de?F(t,":not(."+n.options.selectedClass+")"):F(t),i.push({multiDragElement:t,index:e})}),{items:e(se),clones:[].concat(ce),oldIndicies:o,newIndicies:i}},optionListeners:{multiDragKey:function(t){return"ctrl"===(t=t.toLowerCase())?t="Control":1<t.length&&(t=t.charAt(0).toUpperCase()+t.substr(1)),t}}})}),Rt});
// ── webaudio-controls.js ───
/* *
 *
 *  WebAudio-Controls is based on
 *    webaudio-knob by Eiji Kitamura http://google.com/+agektmr
 *    webaudio-slider by RYoya Kawai https://plus.google.com/108242669191458983485/posts
 *    webaudio-switch by Keisuke Ai http://d.hatena.ne.jp/aike/
 *  Integrated and enhanced by g200kg http://www.g200kg.com/
 *
 *	Copyright 2013 Eiji Kitamura / Ryoya KAWAI / Keisuke Ai / g200kg(Tatsuya Shinyagaito)
 *
 *	 Licensed under the Apache License, Version 2.0 (the "License");
 *	 you may not use this file except in compliance with the License.
 *	 You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 *	 Unless required by applicable law or agreed to in writing, software
 *	 distributed under the License is distributed on an "AS IS" BASIS,
 *	 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	 See the License for the specific language governing permissions and
 *	 limitations under the License.
 *
 * */
if(window.customElements){
  let styles=document.createElement("style");
  styles.innerHTML=
`#webaudioctrl-context-menu {
  display: none;
  position: absolute;
  z-index: 10;
  padding: 0;
  width: 100px;
  color:#eee;
  background-color: #268;
  border: solid 1px #888;
  box-shadow: 1px 1px 2px #888;
  font-family: sans-serif;
  font-size: 11px;
  line-height:1.7em;
  text-align:center;
  cursor:pointer;
  color:#fff;
  list-style: none;
}
#webaudioctrl-context-menu.active {
  display: block;
}
.webaudioctrl-context-menu__item {
  display: block;
  margin: 0;
  padding: 0;
  color: #000;
  background-color:#eee;
  text-decoration: none;
}
.webaudioctrl-context-menu__title{
  font-weight:bold;
}
.webaudioctrl-context-menu__item:last-child {
  margin-bottom: 0;
}
.webaudioctrl-context-menu__item:hover {
  background-color: #b8b8b8;
}
`;
  document.head.appendChild(styles);
  let midimenu=document.createElement("ul");
  midimenu.id="webaudioctrl-context-menu";
  midimenu.innerHTML=
`<li class="webaudioctrl-context-menu__title">MIDI Learn</li>
<li class="webaudioctrl-context-menu__item" id="webaudioctrl-context-menu-learn" onclick="webAudioControlsWidgetManager.contextMenuLearn()">Learn</li>
<li class="webaudioctrl-context-menu__item" onclick="webAudioControlsWidgetManager.contextMenuClear()">Clear</li>
<li class="webaudioctrl-context-menu__item" onclick="webAudioControlsWidgetManager.contextMenuClose()">Close</li>
`;
  let opt={
    useMidi:0,
    bubbles:true,
    cancelable:true,
    preserveMidiLearn:0,
    preserveValue:0,
    midilearn:0,
    mididump:0,
    outline:null,
    knobSrc:null,
    knobSprites:null,
    knobWidth:null,
    knobHeight:null,
    knobDiameter:null,
    knobColors:"#e00;#000;#fff",
    sliderSrc:null,
    sliderWidth:null,
    sliderHeight:null,
    sliderKnobSrc:null,
    sliderKnobWidth:null,
    sliderKnobHeight:null,
    sliderDitchlength:null,
    sliderColors:"#e00;#333;#fcc",
    switchWidth:null,
    switchHeight:null,
    switchDiameter:null,
    switchColors:"#e00;#000;#fcc",
    paramWidth:null,
    paramHeight:null,
    paramFontSize:9,
    paramColors:"#fff;#000",
    valuetip:0,
    xypadColors:"#e00;#000;#fcc",
  };
  if(window.WebAudioControlsOptions)
    Object.assign(opt,window.WebAudioControlsOptions);
  class WebAudioControlsWidget extends HTMLElement{
    constructor(){
      super();
      this.addEventListener("keydown",this.keydown);
      this.addEventListener("mousedown",this.pointerdown,{passive:false});
      this.addEventListener("touchstart",this.pointerdown,{passive:false});
      this.addEventListener("wheel",this.wheel,{passive:false});
      this.addEventListener("mouseover",this.pointerover);
      this.addEventListener("mouseout",this.pointerout);
      this.addEventListener("contextmenu",this.contextMenu);
      this.hover=this.drag=0;
      document.body.appendChild(midimenu);
      this.basestyle=`
.webaudioctrl-tooltip{
  display:inline-block;
  position:absolute;
  margin:0 -1000px;
  z-index: 999;
  background:#eee;
  color:#000;
  border:1px solid #666;
  border-radius:4px;
  padding:5px 10px;
  text-align:center;
  left:0; top:0;
  font-size:11px;
  opacity:0;
  visibility:hidden;
}
.webaudioctrl-tooltip:before{
  content: "";
  position: absolute;
  top: 100%;
  left: 50%;
  margin-left: -8px;
  border: 8px solid transparent;
  border-top: 8px solid #666;
}
.webaudioctrl-tooltip:after{
  content: "";
  position: absolute;
  top: 100%;
  left: 50%;
  margin-left: -6px;
  border: 6px solid transparent;
  border-top: 6px solid #eee;
}
`;
      this.onblur=()=>{
        this.elem.style.outline="none";
      }
      this.onfocus=()=>{
        switch(+this.outline){
        case null:
        case 0:
          this.elem.style.outline="none";
          break;
        case 1:
          this.elem.style.outline="1px solid #444";
          break;
        default:
          this.elem.style.outline=this.outline;
        }
      }
    }
    sendEvent(ev){
      let event = new Event(ev, {bubbles:opt.bubbles?true:false, cancelable:opt.cancelable?true:false});
      this.dispatchEvent(event);
    }
    getAttr(n,def){
      let v=this.getAttribute(n);
      if(v==null) return def;
      switch(typeof(def)){
      case "number":
        if(v=="true") return 1;
        v=+v;
        if(isNaN(v)) return 0;
        return v;
      }
      return v;
    }
    showtip(d){
      function valstr(x,c,type){
        switch(type){
        case "x": return (x|0).toString(16);
        case "X": return (x|0).toString(16).toUpperCase();
        case "d": return (x|0).toString();
        case "f": return parseFloat(x).toFixed(c);
        case "s": return x.toString();
        }
        return "";
      }
      function numformat(s,x){
        let i=s.indexOf("%");
        let j=i+1;
        if(i<0)
          j=s.length;
        let c=[0,0],type=0,m=0,r="";
        if(s.indexOf("%s")>=0){
          return s.replace("%s",x);
        }
        for(;j<s.length;++j){
          if("dfxXs".indexOf(s[j])>=0){
            type=s[j];
            break;
          }
          if(s[j]==".")
            m=1;
          else
            c[m]=c[m]*10+parseInt(s[j]);
        }
        r=valstr(x,c[1],type);
        if(c[0]>0)
          r=("               "+r).slice(-c[0]);
        r=s.replace(/%.*[xXdfs]/,r);
        return r;
      }
      let s=this.tooltip;
      if(this.drag||this.hover){
        if(this.valuetip){
          if(s==null)
            s=`%s`;
          else if(s.indexOf("%")<0)
            s+=` : %s`;
        }
        if(s){
          this.ttframe.innerHTML=numformat(s,this.convValue);
          this.ttframe.style.display="inline-block";
          this.ttframe.style.width="auto";
          this.ttframe.style.height="auto";
          this.ttframe.style.transition="opacity 0.5s "+d+"s,visibility 0.5s "+d+"s";
          this.ttframe.style.opacity=0.9;
          this.ttframe.style.visibility="visible";
          let rc=this.getBoundingClientRect(),rc2=this.ttframe.getBoundingClientRect(),rc3=document.documentElement.getBoundingClientRect();
          this.ttframe.style.left=((rc.width-rc2.width)*0.5+1000)+"px";
          this.ttframe.style.top=(-rc2.height-8)+"px";
          return;
        }
      }
      this.ttframe.style.transition="opacity 0.1s "+d+"s,visibility 0.1s "+d+"s";
      this.ttframe.style.opacity=0;
      this.ttframe.style.visibility="hidden";
    }
    setupLabel(){
      this.labelpos=this.getAttr("labelpos", "bottom 0px");
      const lpos=this.labelpos.split(" ");
      let offs="";
      if(lpos.length==3)
        offs=`translate(${lpos[1]},${lpos[2]})`;
      this.label.style.position="absolute";
      switch(lpos[0]){
      case "center":
        this.label.style.top="50%";
        this.label.style.left="50%";
        this.label.style.transform=`translate(-50%,-50%) ${offs}`;
        break;
      case "right":
        this.label.style.top="50%";
        this.label.style.left="100%";
        this.label.style.transform=`translateY(-50%) ${offs}`;
        break;
      case "left":
        this.label.style.top="50%";
        this.label.style.left="0%";
        this.label.style.transform=`translate(-100%,-50%) ${offs}`;
        break;
      case "bottom":
        this.label.style.top="100%";
        this.label.style.left="50%";
        this.label.style.transform=`translateX(-50%) ${offs}`;
        break;
      case "top":
        this.label.style.top="0%";
        this.label.style.left="50%";
        this.label.style.transform=`translate(-50%,-100%) ${offs}`;
        break;
      }
    }
    pointerover(e) {
      this.hover=1;
      this.showtip(0.6);
    }
    pointerout(e) {
      this.hover=0;
      this.showtip(0);
    }
    contextMenu(e){
      if(window.webAudioControlsWidgetManager && this.midilearn)
        webAudioControlsWidgetManager.contextMenuOpen(e,this);
      e.preventDefault();
      e.stopPropagation();
    }
    setMidiController(channel, cc) {
      if (this.listeningToThisMidiController(channel, cc)) return;
      this.midiController={ 'channel': channel, 'cc': cc};
      console.log("Added mapping for channel=" + channel + " cc=" + cc + " tooltip=" + this.tooltip);
    }
    listeningToThisMidiController(channel, cc) {
      const c = this.midiController;
      if((c.channel === channel || c.channel < 0) && c.cc === cc)
        return true;
      return false;
    }
    processMidiEvent(event){
      const channel = event.data[0] & 0xf;
      const controlNumber = event.data[1];
      if(this.midiMode == 'learn') {
        this.setMidiController(channel, controlNumber);
        webAudioControlsWidgetManager.contextMenuClose();
        this.midiMode = 'normal';
        webAudioControlsWidgetManager.preserveMidiLearn();
      }
      if(this.listeningToThisMidiController(channel, controlNumber)) {
        if(this.tagName=="WEBAUDIO-SWITCH"){
          switch(this.type){
          case "toggle":
            if(event.data[2]>=64)
              this.setValue(1-this.value,true);
            break;
          case "kick":
            this.setValue(event.data[2]>=64?1:0);
            break;
          case "radio":
            let els=document.querySelectorAll("webaudio-switch[type='radio'][group='"+this.group+"']");
            for(let i=0;i<els.length;++i){
              if(els[i]==this)
                els[i].setValue(1);
              else
                els[i].setValue(0);
            }
            break;
          }
        }
        else{
          const val = this.min+(this.max-this.min)*event.data[2]/127;
          this.setValue(val, true);
        }
      }
    }
  }

try{
    customElements.define("webaudio-knob", class WebAudioKnob extends WebAudioControlsWidget {
    constructor(){
      super();
    }
    connectedCallback(){
      let root;
      if(this.attachShadow)
        root=this.attachShadow({mode: 'open'});
      else
        root=this;
      root.innerHTML=
`<style>
${this.basestyle}
:host{
  display:inline-block;
  margin:0;
  padding:0;
  cursor:pointer;
  font-family: sans-serif;
  font-size: 11px;
}
.webaudio-knob-body{
  display:inline-block;
  position:relative;
  margin:0;
  padding:0;
  vertical-align:bottom;
  white-space:pre;
}
</style>
<div class='webaudio-knob-body' tabindex='1' touch-action='none'><div class='webaudioctrl-tooltip'></div><div part="label" class="webaudioctrl-label"><slot></slot></div></div>
`;
      this.elem=root.childNodes[2];
      this.ttframe=this.elem.firstChild;
      this.label=this.ttframe.nextSibling;
      this.enable=this.getAttr("enable",1);
      this._src=this.getAttr("src",opt.knobSrc); if (!this.hasOwnProperty("src")) Object.defineProperty(this,"src",{get:()=>{return this._src},set:(v)=>{this._src=v;this.setupImage()}});
      this._value=this.getAttr("value",0); if (!this.hasOwnProperty("value")) Object.defineProperty(this,"value",{get:()=>{return this._value},set:(v)=>{this._value=v;this.redraw()}});
      this.defvalue=this.getAttr("defvalue",this._value);
      this._min=this.getAttr("min",0); if (!this.hasOwnProperty("min")) Object.defineProperty(this,"min",{get:()=>{return this._min},set:(v)=>{this._min=+v;this.redraw()}});
      this._max=this.getAttr("max",100); if (!this.hasOwnProperty("max")) Object.defineProperty(this,"max",{get:()=>{return this._max},set:(v)=>{this._max=+v;this.redraw()}});
      this._step=this.getAttr("step",1); if (!this.hasOwnProperty("step")) Object.defineProperty(this,"step",{get:()=>{return this._step},set:(v)=>{this._step=+v;this.redraw()}});
      this._sprites=this.getAttr("sprites",opt.knobSprites); if (!this.hasOwnProperty("sprites")) Object.defineProperty(this,"sprites",{get:()=>{return this._sprites},set:(v)=>{this._sprites=v;this.setupImage()}});
      this._width=this.getAttr("width", null); if (!this.hasOwnProperty("width")) Object.defineProperty(this,"width",{get:()=>{return this._width},set:(v)=>{this._width=v;this.setupImage()}});
      this._height=this.getAttr("height", null); if (!this.hasOwnProperty("height")) Object.defineProperty(this,"height",{get:()=>{return this._height},set:(v)=>{this._height=v;this.setupImage()}});
      this._diameter=this.getAttr("diameter", null); if (!this.hasOwnProperty("diameter")) Object.defineProperty(this,"diameter",{get:()=>{return this._diameter},set:(v)=>{this._diameter=v;this.setupImage()}});
      this._colors=this.getAttr("colors",opt.knobColors); if (!this.hasOwnProperty("colors")) Object.defineProperty(this,"colors",{get:()=>{return this._colors},set:(v)=>{this._colors=v;this.setupImage()}});
      this.outline=this.getAttr("outline",opt.outline);
      this.setupLabel();
      this.log=this.getAttr("log",0);
      this.sensitivity=this.getAttr("sensitivity",1);
      this.valuetip=this.getAttr("valuetip",opt.valuetip);
      this.tooltip=this.getAttr("tooltip",null);
      this.conv=this.getAttr("conv",null);
      if(this.conv){
        const x=this._value;
        this.convValue=eval(this.conv);
        if(typeof(this.convValue)=="function")
          this.convValue=this.convValue(x);
      }
      else
        this.convValue=this._value;
      this.midilearn=this.getAttr("midilearn",opt.midilearn);
      this.midicc=this.getAttr("midicc",null);
      this.midiController={};
      this.midiMode="normal";
      if(this.midicc) {
          let ch = parseInt(this.midicc.substring(0, this.midicc.lastIndexOf("."))) - 1;
          let cc = parseInt(this.midicc.substring(this.midicc.lastIndexOf(".") + 1));
          this.setMidiController(ch, cc);
      }
      if(this.midilearn && this.id){
        if(webAudioControlsWidgetManager && webAudioControlsWidgetManager.midiLearnTable){
          const ml=webAudioControlsWidgetManager.midiLearnTable;
          for(let i=0; i < ml.length; ++i){
            if(ml[i].id==this.id){
              this.setMidiController(ml[i].cc.channel, ml[i].cc.cc);
              break;
            }
          }
        }
      }
      this.setupImage();
      this.digits=0;
      if(this.step && this.step < 1) {
        for(let n = this.step ; n < 1; n *= 10)
          ++this.digits;
      }
      this._setValue(this._value);
      this.coltab=["#e00","#000","#000"];
      if(window.webAudioControlsWidgetManager)
        window.webAudioControlsWidgetManager.addWidget(this);
    }
    disconnectedCallback(){}
    setupImage(){
      this.kw=this._width||this._diameter||opt.knobWidth||opt.knobDiameter;
      this.kh=this._height||this._diameter||opt.knobHeight||opt.knobDiameter;
      if(!this.src){
        if(this.colors)
          this.coltab = this.colors.split(";");
        if(!this.coltab)
          this.coltab=["#e00","#000","#000"];
        let svg=
`<svg xmlns="http://www.w3.org/2000/svg" width="64" height="6464" preserveAspectRatio="none">
<defs>
  <filter id="f1">
    <feGaussianBlur in="SourceGraphic" stdDeviation="0.8" />
  </filter>
  <radialGradient id="g1" cx="50%" cy="50%">
    <stop offset="0%" stop-color="${this.coltab[2]}"/>
    <stop offset="100%" stop-color="${this.coltab[1]}"/>
  </radialGradient>
  <linearGradient id="g2" x1="0%" y1="0%" x2="0%" y2="100%">
    <stop offset="0%" stop-color="#000" stop-opacity="0"/>
    <stop offset="100%" stop-color="#000" stop-opacity="0.08"/>
  </linearGradient>
  <g id="B">
    <circle cx="32" cy="32" r="30" fill="${this.coltab[1]}" opacity="0.15"/>
    <circle cx="32" cy="32" r="29" fill="url(#g1)"/>
    <circle cx="32" cy="32" r="29" fill="url(#g2)"/>
  </g>
  <line id="K" x1="32" y1="24" x2="32" y2="8" stroke-linecap="butt" stroke-width="3" stroke="${this.coltab[0]}"/>
</defs>`;
        for(let i=0;i<101;++i){
          svg += `<use href="#B" y="${64*i}"/><use href="#K" y="${64*i}" transform="rotate(${(-135+270*i/101).toFixed(2)},32,${64*i+32})"/>`;
        }
        svg += "</svg>";
        this.elem.style.backgroundImage = "url(data:image/svg+xml;base64,"+btoa(svg)+")";
        if(this.kw==null) this.kw=64;
        if(this.kh==null) this.kh=64;
        this.elem.style.backgroundSize = `${this.kw}px ${this.kh*101}px`;
        this.elem.style.width=this.kw+"px";
        this.elem.style.height=this.kh+"px";
        this.style.height=this.kh+"px";
        this.fireflag=true;
        this.redraw();
        return;
      }
      else{
        this.img=new Image();
        this.img.onload=()=>{
          this.elem.style.backgroundImage = "url("+(this.src)+")";
          if(this._sprites==null)
            this._sprites=this.img.height/this.img.width - 1;
          else
            this._sprites=+this._sprites;
          if(this.kw==null) this.kw=this.img.width;
          if(this.kh==null) this.kh=this.img.height/(this.sprites+1);
          if(!this.sprites)
            this.elem.style.backgroundSize = "100% 100%";
          else
            this.elem.style.backgroundSize = `${this.kw}px ${this.kh*(this.sprites+1)}px`;
          this.elem.style.width=this.kw+"px";
          this.elem.style.height=this.kh+"px";
          this.style.height=this.kh+"px";
          this.redraw();
        };
        this.img.src=this.src;
      }
    }
    redraw() {
      let ratio;
      this.digits=0;
      if(this.step && this.step < 1) {
        for(let n = this.step ; n < 1; n *= 10)
          ++this.digits;
      }
      if(this.value<this.min){
        this.value=this.min;
      }
      if(this.value>this.max){
        this.value=this.max;
      }
      if(this.log)
        ratio = Math.log(this.value/this.min) / Math.log(this.max/this.min);
      else
        ratio = (this.value - this.min) / (this.max - this.min);
      let style = this.elem.style;
      let sp = this.src?this.sprites:100;
      if(sp>=1){
        let offset = (sp * ratio) | 0;
        style.backgroundPosition = "0px " + (-offset*this.kh) + "px";
        style.transform = 'rotate(0deg)';
      } else {
        let deg = 270 * (ratio - 0.5);
        style.backgroundPosition="0px 0px";
        style.transform = 'rotate(' + deg + 'deg)';
      }
    }
    _setValue(v){
      if(this.step)
        v=(Math.round((v-this.min)/this.step))*this.step+this.min;
      this._value=Math.min(this.max,Math.max(this.min,v));
      if(this._value!=this.oldvalue){
        this.fireflag=true;
        this.oldvalue=this._value;
        if(this.conv){
          const x=this._value;
          this.convValue=eval(this.conv);
          if(typeof(this.convValue)=="function")
            this.convValue=this.convValue(x);
        }
        else
          this.convValue=this._value;
        if(typeof(this.convValue)=="number"){
          this.convValue=this.convValue.toFixed(this.digits);
        }
        this.redraw();
        this.showtip(0);
        return 1;
      }
      return 0;
    }
    setValue(v,f){
      if(this._setValue(v) && f)
        this.sendEvent("input"),this.sendEvent("change");
    }
    keydown(e){
      const delta = this.step;
      if(delta==0)
        delta=1;
      switch(e.key){
      case "ArrowUp":
        this.setValue(this.value+delta,true);
        break;
      case "ArrowDown":
        this.setValue(this.value-delta,true);
        break;
      default:
          return;
      }
      e.preventDefault();
      e.stopPropagation();
    }
    wheel(e) {
      if (!this.enable)
        return;
      if(this.log){
        let r=Math.log(this.value/this.min)/Math.log(this.max/this.min);
        let d = (e.deltaY>0?-0.01:0.01);
        if(!e.shiftKey)
          d*=5;
        r += d;
        this.setValue(this.min*Math.pow(this.max/this.min,r),true);
      }
      else{
        let delta=Math.max(this.step, (this.max-this.min)*0.05);
        if(e.shiftKey)
          delta=this.step?this.step:1;
        delta=e.deltaY>0?-delta:delta;
        this.setValue(+this.value+delta,true);
      }
      e.preventDefault();
      e.stopPropagation();
    }
    pointerdown(ev){
      if(!this.enable)
        return;
      let e=ev;
      if(ev.touches){
        e = ev.changedTouches[0];
        this.identifier=e.identifier;
      }
      else {
        if(e.buttons!=1 && e.button!=0)
          return;
      }
      this.elem.focus();
      this.drag=1;
      this.showtip(0);
      this.oldvalue=this._value;
      let pointermove=(ev)=>{
        let e=ev;
        if(ev.touches){
          for(let i=0;i<ev.touches.length;++i){
            if(ev.touches[i].identifier==this.identifier){
              e = ev.touches[i];
              break;
            }
          }
        }
        if(this.lastShift !== e.shiftKey) {
          this.lastShift = e.shiftKey;
          this.startPosX = e.pageX;
          this.startPosY = e.pageY;
          this.startVal = this.value;
        }
        let offset = (this.startPosY - e.pageY - this.startPosX + e.pageX) * this.sensitivity;
        if(this.log){
          let r = Math.log(this.startVal / this.min) / Math.log(this.max / this.min);
          r += offset/((e.shiftKey?4:1)*128);
          if(r<0) r=0;
          if(r>1) r=1;
          this._setValue(this.min * Math.pow(this.max/this.min, r));
        }
        else{
          this._setValue(this.min + ((((this.startVal + (this.max - this.min) * offset / ((e.shiftKey ? 4 : 1) * 128)) - this.min) / this.step) | 0) * this.step);
        }
        if(this.fireflag){
          this.sendEvent("input");
          this.fireflag=false;
        }
        if(e.preventDefault)
          e.preventDefault();
        if(e.stopPropagation)
          e.stopPropagation();
        return false;
      }
      let pointerup=(ev)=>{
        let e=ev;
        if(ev.touches){
          for(let i=0;;){
            if(ev.changedTouches[i].identifier==this.identifier){
              break;
            }
            if(++i>=ev.changedTouches.length)
              return;
          }
        }
        this.drag=0;
        this.showtip(0);
        this.startPosX = this.startPosY = null;
        window.removeEventListener('mousemove', pointermove);
        window.removeEventListener('touchmove', pointermove, {passive:false});
        window.removeEventListener('mouseup', pointerup);
        window.removeEventListener('touchend', pointerup);
        window.removeEventListener('touchcancel', pointerup);
        document.body.removeEventListener('touchstart', preventScroll,{passive:false});
        this.sendEvent("change");
      }
      let preventScroll=(e)=>{
        e.preventDefault();
      }
      if(e.ctrlKey || e.metaKey)
        this.setValue(this.defvalue,true);
      else {
        this.startPosX = e.pageX;
        this.startPosY = e.pageY;
        this.startVal = this.value;
        window.addEventListener('mousemove', pointermove);
        window.addEventListener('touchmove', pointermove, {passive:false});
      }
      window.addEventListener('mouseup', pointerup);
      window.addEventListener('touchend', pointerup);
      window.addEventListener('touchcancel', pointerup);
      document.body.addEventListener('touchstart', preventScroll,{passive:false});
      ev.preventDefault();
      ev.stopPropagation();
      return false;
    }
  });
} catch(error){
  console.log("webaudio-knob already defined");
}

try{
  customElements.define("webaudio-slider", class WebAudioSlider extends WebAudioControlsWidget {
    constructor(){
      super();
    }
    connectedCallback(){
      let root;
      if(this.attachShadow)
        root=this.attachShadow({mode: 'open'});
      else
        root=this;
      root.innerHTML=
`<style>
${this.basestyle}
:host{
  display:inline-block;
  position:relative;
  margin:0;
  padding:0;
  font-family: sans-serif;
  font-size: 11px;
  cursor:pointer;
}
.webaudio-slider-body{
  display:inline-block;
  position:relative;
  margin:0;
  padding:0;
  vertical-align:bottom;
  white-space:pre;
}
.webaudio-slider-knob{
  display:inline-block;
  position:absolute;
  margin:0;
  padding:0;
}
</style>
<div class='webaudio-slider-body' tabindex='1' touch-action='none'><div class='webaudio-slider-knob' touch-action='none'></div><div class='webaudioctrl-tooltip'></div><div part="label" class="webaudioctrl-label"><slot></slot></div></div>
`;
      this.elem=root.childNodes[2];
      this.knob=this.elem.firstChild;
      this.ttframe=this.knob.nextSibling;
      this.label=this.ttframe.nextSibling;
      this.enable=this.getAttr("enable",1);
      this.tracking=this.getAttr("tracking","rel"); 
      this._src=this.getAttr("src",opt.sliderSrc); if (!this.hasOwnProperty("src")) Object.defineProperty(this,"src",{get:()=>{return this._src},set:(v)=>{this._src=v;this.setupImage()}});
      this._knobsrc=this.getAttr("knobsrc",opt.sliderKnobSrc); if (!this.hasOwnProperty("knobsrc")) Object.defineProperty(this,"knobsrc",{get:()=>{return this._knobsrc},set:(v)=>{this._knobsrc=v;this.setupImage()}});
      this._value=this.getAttr("value",0); if (!this.hasOwnProperty("value")) Object.defineProperty(this,"value",{get:()=>{return this._value},set:(v)=>{this._value=v;this.redraw()}});
      this.defvalue=this.getAttr("defvalue",this._value);
      this._min=this.getAttr("min",0); if (!this.hasOwnProperty("min")) Object.defineProperty(this,"min",{get:()=>{return this._min},set:(v)=>{this._min=v;this.redraw()}});
      this._max=this.getAttr("max",100); if (!this.hasOwnProperty("max")) Object.defineProperty(this,"max",{get:()=>{return this._max},set:(v)=>{this._max=v;this.redraw()}});
      this._step=this.getAttr("step",1); if (!this.hasOwnProperty("step")) Object.defineProperty(this,"step",{get:()=>{return this._step},set:(v)=>{this._step=v;this.redraw()}});
      this._sprites=this.getAttr("sprites",0); if (!this.hasOwnProperty("sprites")) Object.defineProperty(this,"sprites",{get:()=>{return this._sprites},set:(v)=>{this._sprites=v;this.setupImage()}});
      this._direction=this.getAttr("direction",null); if (!this.hasOwnProperty("direction")) Object.defineProperty(this,"direction",{get:()=>{return this._direction},set:(v)=>{this._direction=v;this.setupImage()}});
      this.log=this.getAttr("log",0);
      this._width=this.getAttr("width",opt.sliderWidth); if (!this.hasOwnProperty("width")) Object.defineProperty(this,"width",{get:()=>{return this._width},set:(v)=>{this._width=v;this.setupImage()}});
      this._height=this.getAttr("height",opt.sliderHeight); if (!this.hasOwnProperty("height")) Object.defineProperty(this,"height",{get:()=>{return this._height},set:(v)=>{this._height=v;this.setupImage()}});
      this._knobwidth=this.getAttr("knobwidth",opt.sliderKnobWidth); if (!this.hasOwnProperty("knobwidth")) Object.defineProperty(this,"knobwidth",{get:()=>{return this._knobwidth},set:(v)=>{this._knobwidth=v;this.setupImage()}});
      this._knobheight=this.getAttr("knobheight",opt.sliderKnobHeight); if (!this.hasOwnProperty("knobheight")) Object.defineProperty(this,"knobheight",{get:()=>{return this._knobheight},set:(v)=>{this._knobheight=v;this.setupImage()}});
      this._ditchlength=this.getAttr("ditchlength",opt.sliderDitchlength); if (!this.hasOwnProperty("ditchlength")) Object.defineProperty(this,"ditchlength",{get:()=>{return this._ditchlength},set:(v)=>{this._ditchlength=v;this.setupImage()}});
      this._colors=this.getAttr("colors",opt.sliderColors); if (!this.hasOwnProperty("colors")) Object.defineProperty(this,"colors",{get:()=>{return this._colors},set:(v)=>{this._colors=v;this.setupImage()}});
      this.outline=this.getAttr("outline",opt.outline);
      this.setupLabel();
      this.sensitivity=this.getAttr("sensitivity",1);
      this.valuetip=this.getAttr("valuetip",opt.valuetip);
      this.tooltip=this.getAttr("tooltip",null);
      this.conv=this.getAttr("conv",null);
      if(this.conv){
        const x=this._value;
        this.convValue=eval(this.conv);
        if(typeof(this.convValue)=="function")
          this.convValue=this.convValue(x);
      }
      else
        this.convValue=this._value;
      this.midilearn=this.getAttr("midilearn",opt.midilearn);
      this.midicc=this.getAttr("midicc",null);
      this.midiController={};
      this.midiMode="normal";
      if(this.midicc) {
          let ch = parseInt(this.midicc.substring(0, this.midicc.lastIndexOf("."))) - 1;
          let cc = parseInt(this.midicc.substring(this.midicc.lastIndexOf(".") + 1));
          this.setMidiController(ch, cc);
      }
      if(this.midilearn && this.id){
        if(webAudioControlsWidgetManager && webAudioControlsWidgetManager.midiLearnTable){
          const ml=webAudioControlsWidgetManager.midiLearnTable;
          for(let i=0; i < ml.length; ++i){
            if(ml[i].id==this.id){
              this.setMidiController(ml[i].cc.channel, ml[i].cc.cc);
              break;
            }
          }
        }
      }
      this.setupImage();
      this.digits=0;
      if(this.step && this.step < 1) {
        for(let n = this.step ; n < 1; n *= 10)
          ++this.digits;
      }
      this.fireflag=true;
      if(window.webAudioControlsWidgetManager)
//        window.webAudioControlsWidgetManager.updateWidgets();
        window.webAudioControlsWidgetManager.addWidget(this);
      this.elem.onclick=(e)=>{e.stopPropagation()};
    }
    disconnectedCallback(){}
    setupImage(){
      this.coltab = this.colors.split(";");
      this.bodyimg=new Image();
      this.knobimg=new Image();
      this.srcurl=null;
      if(this.src==null||this.src==""){
        this.sw=+this._width;
        this.sh=+this.height;
        if(this._direction=="horz"){
          if(this._width==null) this.sw=128;
          if(this._height==null) this.sh=24;
        }
        else if(this._direction=="vert"){
          if(this._width==null) this.sw=24;
          if(this._height==null) this.sh=128;
        }
        else{
          if(this._width==null) this.sw=128;
          if(this._height==null) this.sh=24;
        }
        const r=Math.min(this.sw,this.sh)*0.5;
        const svgbody=
`<svg xmlns="http://www.w3.org/2000/svg" width="${this.sw}" height="${this.sh}" preserveAspectRatio="none">
<defs>
  <filter id="f1">
    <feGaussianBlur in="SourceGraphic" stdDeviation="0.8" />
  </filter>
  <linearGradient id="g1" x1="0%" y1="0%" ${(this.sw>this.sh)?'x2="0%" y2="100%"':'x2="100%" y2="0%"'}>
    <stop offset="0%" stop-color="#000" stop-opacity="0"/>
    <stop offset="100%" stop-color="#000" stop-opacity="0.3"/>
  </linearGradient>
</defs>
<rect x="1" y="1" rx="${r}" ry="${r}" width="${this.sw-2}" height="${this.sh-2}" fill="#000"/>
<rect x="3" y="3" rx="${r}" ry="${r}" width="${this.sw-6}" height="${this.sh-6}" fill="${this.coltab[1]}" filter="url(#f1)"/>
<rect x="1" y="1" rx="${r}" ry="${r}" width="${this.sw-2}" height="${this.sh-2}" fill="url(#g1)"/>
</svg>`;
        this.srcurl = "data:image/svg+xml;base64,"+btoa(svgbody);
      }
      else{
        this.srcurl = this.src;
      }
      this.bodyimg.onload=()=>{
        if(this.src!="")
          this.elem.style.backgroundImage = "url("+this.srcurl+")";
        this.sw=+this._width;
        this.sh=+this._height;
        if(this._width==null) this.sw=this.bodyimg.width;
        if(this._height==null) this.sh=this.bodyimg.height;
        if(this.dr==null){
          if(this.sw>this.sh)
            this.dr="horz";
          else
            this.dr="vert";
        }
        this.kw=+this._knobwidth;
        this.kh=+this._knobheight;
        if(this._knobsrc==null){
          if(this._knobwidth==null) this.kw=Math.min(this.sw,this.sh);
          if(this._knobheight==null) this.kh=Math.min(this.sw,this.sh);
          const mm=Math.min(this.kw,this.kh)*0.5;
          const kw2=Math.max(1,this.kw-12);
          const kh2=Math.max(1,this.kh-12);
          const svgknob=
`<svg xmlns="http://www.w3.org/2000/svg" width="${this.kw}" height="${this.kh}" preserveAspectRatio="none">
<defs>
  <filter id="f1">
    <feGaussianBlur in="SourceGraphic" stdDeviation="0.8" />
  </filter>
  <linearGradient id="g1" x1="0%" y1="0%" x2="0%" y2="100%">
    <stop offset="0%" stop-color="${this.coltab[2]}"/>
    <stop offset="50%" stop-color="${this.coltab[0]}"/>
    <stop offset="100%" stop-color="${this.coltab[0]}" stop-opacity="0.5"/>
  </linearGradient>
  <linearGradient id="g2" x1="0%" y1="0%" x2="0%" y2="100%">
    <stop offset="0%" stop-color="${this.coltab[0]}"/>
    <stop offset="100%" stop-color="${this.coltab[0]}"/>
  </linearGradient>
  <linearGradient id="g3" x1="0%" y1="0%" x2="0%" y2="100%">
    <stop offset="0%" stop-color="#000" stop-opacity="0"/>
    <stop offset="100%" stop-color="#000" stop-opacity="0.3"/>
  </linearGradient>
</defs>
<rect x="2" y="2" width="${this.kw-4}" height="${this.kh-4}" rx="${mm}" ry="${mm}" fill="#000"/>
<rect x="3" y="3" width="${this.kw-6}" height="${this.kh-6}" rx="${mm}" ry="${mm}" fill="url(#g1)"/>
<rect x="6" y="6" width="${kw2}" height="${kh2}" rx="${mm}" ry="${mm}" fill="url(#g2)" filter="url(#f1)"/>
<rect x="3" y="3" width="${this.kw-6}" height="${this.kh-6}" rx="${mm}" ry="${mm}" fill="url(#g3)"/>
</svg>`;
          this.knobsrcurl = "data:image/svg+xml;base64,"+btoa(svgknob);
        }
        else{
          this.knobsrcurl = this.knobsrc;
        }
        this.knobimg.onload=()=>{
          this.knob.style.backgroundImage = "url("+this.knobsrcurl+")";
          if(this._knobwidth==null) this.kw=this.knobimg.width;
          if(this._knobheight==null) this.kh=this.knobimg.height;
          this.dlen=this.ditchlength;
          if(this.dlen==null){
            if(this.dr=="horz")
              this.dlen=this.sw-this.kw;
            else
              this.dlen=this.sh-this.kh;
          }
          this.knob.style.backgroundSize = "100% 100%";
          this.knob.style.width = this.kw+"px";
          this.knob.style.height = this.kh+"px";
          this.elem.style.backgroundSize = "100% 100%";
          this.elem.style.width=this.sw+"px";
          this.elem.style.height=this.sh+"px";
          this.redraw();
        };
        this.knobimg.src=this.knobsrcurl;
      };
      this.bodyimg.src=this.srcurl;
    }
    redraw() {
      let ratio;
      this.digits=0;
      if(this.step && this.step < 1) {
        for(let n = this.step ; n < 1; n *= 10)
          ++this.digits;
      }
      if(this.value<this.min){
        this.value=this.min;
      }
      if(this.value>this.max){
        this.value=this.max;
      }
      if(this.log)
        ratio = Math.log(this.value/this.min) / Math.log(this.max/this.min);
      else
        ratio = (this.value - this.min) / (this.max - this.min);
      let style = this.knob.style;
      if(this.dr=="horz"){
        style.top=(this.sh-this.kh)*0.5+"px";
        style.left=((this.sw-this.kw-this.dlen)*0.5+ratio*this.dlen)+"px";
        this.sensex=1; this.sensey=0;
      }
      else{
        style.left=(this.sw-this.kw)*0.5+"px";
        style.top=((this.sh-this.kh-this.dlen)*0.5+(1-ratio)*this.dlen)+"px";
        this.sensex=0; this.sensey=1;
      }
    }
    _setValue(v){
      v=(Math.round((v-this.min)/this.step))*this.step+this.min;
      this._value=Math.min(this.max,Math.max(this.min,v));
      if(this._value!=this.oldvalue){
        this.oldvalue=this._value;
        this.fireflag=true;
        if(this.conv){
          const x=this._value;
          this.convValue=eval(this.conv);
          if(typeof(this.convValue)=="function")
            this.convValue=this.convValue(x);
        }
        else
          this.convValue=this._value;
        if(typeof(this.convValue)=="number"){
          this.convValue=this.convValue.toFixed(this.digits);
        }
        this.redraw();
        this.showtip(0);
        return 1;
      }
      return 0;
    }
    setValue(v,f){
      if(this._setValue(v)&&f)
        this.sendEvent("input"),this.sendEvent("change");
    }
    keydown(e){
      const delta = this.step;
      if(delta==0)
        delta=1;
      switch(e.key){
      case "ArrowUp":
        this.setValue(this.value+delta,true);
        break;
      case "ArrowDown":
        this.setValue(this.value-delta,true);
        break;
      default:
          return;
      }
      e.preventDefault();
      e.stopPropagation();
    }
    wheel(e) {
      if (!this.enable)
        return;
      if(this.log){
        let r=Math.log(this.value/this.min)/Math.log(this.max/this.min);
        let d = (e.deltaY>0?-0.01:0.01);
        if(!e.shiftKey)
          d*=5;
        r += d;
        this.setValue(this.min*Math.pow(this.max/this.min,r),true);
      }
      else{
        let delta=Math.max(this.step, (this.max-this.min)*0.05);
        if(e.shiftKey)
          delta=this.step?this.step:1;
        delta=e.deltaY>0?-delta:delta;
        this.setValue(+this.value+delta,true);
      }
      e.preventDefault();
      e.stopPropagation();
    }
    pointerdown(ev){
      if(!this.enable)
        return;
      let e=ev;
      if(ev.touches){
        e = ev.changedTouches[0];
        this.identifier=e.identifier;
      }
      else {
        if(e.buttons!=1 && e.button!=0)
          return;
      }
      this.elem.focus();
      this.drag=1;
      this.showtip(0);
      let pointermove=(ev)=>{
        let e=ev;
        if(ev.touches){
          for(let i=0;i<ev.touches.length;++i){
            if(ev.touches[i].identifier==this.identifier){
              e = ev.touches[i];
              break;
            }
          }
        }
        if(this.lastShift !== e.shiftKey) {
          this.lastShift = e.shiftKey;
          this.startPosX = e.pageX;
          this.startPosY = e.pageY;
          this.startVal = this.value;
        }
        if(this.tracking=="abs"){
          const rc = this.getBoundingClientRect();
          let val;
          if(this.dr=="horz")
            val = Math.max(0,Math.min(1,(e.pageX-rc.left-window.pageXOffset-this.kw*0.5)/(this.width-this.kw)));
          else
            val = 1 - Math.max(0,Math.min(1,(e.pageY-rc.top-window.pageYOffset-this.kh*0.5)/(this.height-this.kh)));
          if(this.log){
            this._setValue(this.min * Math.pow(this.max/this.min, val));
          }
          else
            this._setValue(this.min + (this.max - this.min)*val);
        }
        else{
          let offset = ((this.startPosY - e.pageY)*this.sensey - (this.startPosX - e.pageX)*this.sensex) * this.sensitivity;
          if(this.log){
            let r = Math.log(this.startVal / this.min) / Math.log(this.max / this.min);
            r += offset/((e.shiftKey?4:1)*128);
            if(r<0) r=0;
            if(r>1) r=1;
            this._setValue(this.min * Math.pow(this.max/this.min, r));
          }
          else{
            this._setValue(this.min + ((((this.startVal + (this.max - this.min) * offset / ((e.shiftKey ? 4 : 1) * this.dlen)) - this.min) / this.step) | 0) * this.step);
          }
        }
        if(this.fireflag){
          this.sendEvent("input");
          this.fireflag=false;
        }
        if(e.preventDefault)
          e.preventDefault();
        if(e.stopPropagation)
          e.stopPropagation();
        return false;
      }
      let pointerup=(ev)=>{
        let e=ev;
        if(ev.touches){
          for(let i=0;;){
            if(ev.changedTouches[i].identifier==this.identifier){
              break;
            }
            if(++i>=ev.changedTouches.length)
              return;
          }
        }
        this.drag=0;
        this.showtip(0);
        this.startPosX = this.startPosY = null;
        window.removeEventListener('mousemove', pointermove);
        window.removeEventListener('touchmove', pointermove, {passive:false});
        window.removeEventListener('mouseup', pointerup);
        window.removeEventListener('touchend', pointerup);
        window.removeEventListener('touchcancel', pointerup);
        document.body.removeEventListener('touchstart', preventScroll,{passive:false});
        this.sendEvent("change");
      }
      let preventScroll=(e)=>{
        e.preventDefault();
      }
      if(e.touches)
        e = e.touches[0];
      if(e.ctrlKey || e.metaKey)
        this.setValue(this.defvalue,true);
      else {
        this.startPosX = e.pageX;
        this.startPosY = e.pageY;
        this.startVal = this.value;
        window.addEventListener('mousemove', pointermove);
        window.addEventListener('touchmove', pointermove, {passive:false});
        pointermove(ev);
      }
      window.addEventListener('mouseup', pointerup);
      window.addEventListener('touchend', pointerup);
      window.addEventListener('touchcancel', pointerup);
      document.body.addEventListener('touchstart', preventScroll,{passive:false});
      e.preventDefault();
      e.stopPropagation();
      return false;
    }
  });
} catch(error){
  console.log("webaudio-slider already defined");
}

try{
  customElements.define("webaudio-switch", class WebAudioSwitch extends WebAudioControlsWidget {
    constructor(){
      super();
    }
    connectedCallback(){
      let root;
      if(this.attachShadow)
        root=this.attachShadow({mode: 'open'});
      else
        root=this;
      root.innerHTML=
`<style>
${this.basestyle}
:host{
  display:inline-block;
  position:relative;
  margin:0;
  padding:0;
  font-family: sans-serif;
  font-size: 11px;
  cursor:pointer;
}
.webaudio-switch-body{
  display:inline-block;
  position:relative;
  margin:0;
  padding:0;
  vertical-align:bottom;
  white-space:pre;
}
.webaudioctrl-label{
  position:absolute;
  left:50%;
  top:50%;
}
</style>
<div class='webaudio-switch-body' tabindex='1' touch-action='none'><div class='webaudioctrl-tooltip'></div><div part="label" class="webaudioctrl-label"><slot></slot></div></div>
`;
      this.elem=root.childNodes[2];
      this.ttframe=this.elem.firstChild;
      this.label=this.ttframe.nextSibling;
      this.enable=this.getAttr("enable",1);
      this._src=this.getAttr("src",null); if (!this.hasOwnProperty("src")) Object.defineProperty(this,"src",{get:()=>{return this._src},set:(v)=>{this._src=v;this.setupImage()}});
      this._value=this.getAttr("value",0); if (!this.hasOwnProperty("value")) Object.defineProperty(this,"value",{get:()=>{return this._value},set:(v)=>{this._value=v;this.redraw()}});
      this.defvalue=this.getAttr("defvalue",this._value);
      this.type=this.getAttr("type","toggle");
      this.group=this.getAttr("group","");
      this._width=this.getAttr("width",null); if (!this.hasOwnProperty("width")) Object.defineProperty(this,"width",{get:()=>{return this._width},set:(v)=>{this._width=v;this.setupImage()}});
      this._height=this.getAttr("height",null); if (!this.hasOwnProperty("height")) Object.defineProperty(this,"height",{get:()=>{return this._height},set:(v)=>{this._height=v;this.setupImage()}});
      this._diameter=this.getAttr("diameter",null); if (!this.hasOwnProperty("diameter")) Object.defineProperty(this,"diameter",{get:()=>{return this._diameter},set:(v)=>{this._diameter=v;this.setupImage()}});
      this.invert=this.getAttr("invert",0);
      this._colors=this.getAttr("colors",opt.switchColors); if (!this.hasOwnProperty("colors")) Object.defineProperty(this,"colors",{get:()=>{return this._colors},set:(v)=>{this._colors=v;this.setupImage()}});
      this.outline=this.getAttr("outline",opt.outline);
      this.setupLabel();
      this.valuetip=0;
      this.tooltip=this.getAttr("tooltip",null);
      this.midilearn=this.getAttr("midilearn",opt.midilearn);
      this.midicc=this.getAttr("midicc",null);
      this.midiController={};
      this.midiMode="normal";
      if(this.midicc) {
          let ch = parseInt(this.midicc.substring(0, this.midicc.lastIndexOf("."))) - 1;
          let cc = parseInt(this.midicc.substring(this.midicc.lastIndexOf(".") + 1));
          this.setMidiController(ch, cc);
      }
      if(this.midilearn && this.id){
        if(webAudioControlsWidgetManager && webAudioControlsWidgetManager.midiLearnTable){
          const ml=webAudioControlsWidgetManager.midiLearnTable;
          for(let i=0; i < ml.length; ++i){
            if(ml[i].id==this.id){
              this.setMidiController(ml[i].cc.channel, ml[i].cc.cc);
              break;
            }
          }
        }
      }
      this.setupImage();
      this.digits=0;
      if(this.step && this.step < 1) {
        for(let n = this.step ; n < 1; n *= 10)
          ++this.digits;
      }
      if(window.webAudioControlsWidgetManager)
//        window.webAudioControlsWidgetManager.updateWidgets();
        window.webAudioControlsWidgetManager.addWidget(this);
      this.elem.onclick=(e)=>{e.stopPropagation()};
    }
    disconnectedCallback(){}
    setupImage(){
      this.coltab = this.colors.split(";");
      this.kw=this._width||this._diameter||opt.switchWidth||opt.switchDiameter;
      this.kh=this._height||this._diameter||opt.switchHeight||opt.switchDiameter;
      this.img=new Image();
      this.srcurl=null;
      if(this.src==null||this.src==""){
        if(this.kw==null) this.kw=32;
        if(this.kh==null) this.kh=32;
        const mm=Math.min(this.kw,this.kh);
        const kw=this.kw,kh=this.kh;
        const svg=
`<svg xmlns="http://www.w3.org/2000/svg" width="${this.kw}" height="${this.kh*2}" preserveAspectRatio="none">
<defs>
<linearGradient id="g1" x1="0%" y1="0%" x2="0%" y2="100%">
  <stop offset="0%" stop-color="#000" stop-opacity="0"/>
  <stop offset="100%" stop-color="#000" stop-opacity="0.2"/>
</linearGradient>
<radialGradient id="g2" cx="50%" cy="30%">
    <stop offset="0%" stop-color="${this.coltab[2]}"/>
    <stop offset="100%" stop-color="${this.coltab[0]}"/>
  </radialGradient>
  <filter id="f1">
    <feGaussianBlur in="SourceGraphic" stdDeviation=".4" />
  </filter>
</defs>
<g id="p1">
  <rect x="${kw*.075}" y="${kh*.075}" width="${kw*.85}" height="${kh*.85}" rx="${mm*.1}" ry="${mm*.1}" fill="#000"/>
  <rect x="${kw*.1}" y="${kh*.1}" width="${kw*.8}" height="${kh*.8}" rx="${mm*.1}" ry="${mm*.1}" fill="${this.coltab[1]}"/>
</g>
<g id="p2">
  <circle cx="${kw*0.5}" cy="${kh*0.5}" r="${mm*0.35}" stroke="#000" stroke-width="${mm*.03}" fill="${this.coltab[0]}" filter="url(#f1)"/>
  <circle cx="${kw*0.5}" cy="${kh*0.5}" r="${mm*0.27}" stroke="#000" stroke-width="${mm*.03}" fill="#000" filter="url(#f1)"/>
  <rect x="${kw*.075}" y="${kh*.075}" width="${kw*.85}" height="${kh*.85}" rx="${mm*.1}" ry="${mm*.1}" fill="url(#g1)"/>
</g>
<use href="#p1" y="${kh}"/>
<use href="#p2" y="${kh}"/>
<circle cx="${kw*.5}" cy="${kh*1.5}" r="${mm*.25}" fill="url(#g2)" filter="url(#f1)"/>
<circle cx="${kw*.5}" cy="${kh*1.5}" r="${mm*.25}" fill="url(#g1)"/>
</svg>`;
        this.srcurl="data:image/svg+xml;base64,"+btoa(svg);
      }
      else
        this.srcurl=this.src;
      this.img.onload=()=>{
        if(this.kw==null) this.kw=this.img.width;
        if(this.kh==null) this.kh=this.img.height*0.5;
        this.elem.style.backgroundImage = "url("+this.srcurl+")";
        this.elem.style.backgroundSize = "100% 200%";
        this.elem.style.width=this.kw+"px";
        this.elem.style.height=this.kh+"px";
        this.redraw();
      }
      this.img.src=this.srcurl;
    }
    redraw() {
      let style = this.elem.style;
      if(this.value^this.invert)
        style.backgroundPosition = "0px -100%";
      else
        style.backgroundPosition = "0px 0px";
    }
    setValue(v,f){
      this.value=v;
      this.checked=(!!v);
      if(this.value!=this.oldvalue){
        this.redraw();
        this.showtip(0);
        if(f){
          this.sendEvent("input");
          this.sendEvent("change");
        }
        this.oldvalue=this.value;
      }
    }
    pointerdown(ev){
      if(!this.enable)
        return;
      let e=ev;
      if(ev.touches){
        e = ev.changedTouches[0];
        this.identifier=e.identifier;
      }
      else {
        if(e.buttons!=1 && e.button!=0)
          return;
      }
      this.elem.focus();
      this.drag=1;
      this.showtip(0);
      let pointermove=(e)=>{
        e.preventDefault();
        e.stopPropagation();
        return false;
      }
      let pointerup=(e)=>{
        this.drag=0;
        this.showtip(0);
        window.removeEventListener('mousemove', pointermove);
        window.removeEventListener('touchmove', pointermove, {passive:false});
        window.removeEventListener('mouseup', pointerup);
        window.removeEventListener('touchend', pointerup);
        window.removeEventListener('touchcancel', pointerup);
        document.body.removeEventListener('touchstart', preventScroll,{passive:false});
        if(this.type=="kick"){
          this.value=0;
          this.checked=false;
          this.redraw();
          this.sendEvent("change");
        }
        this.sendEvent("click");
        e.preventDefault();
        e.stopPropagation();
      }
      let preventScroll=(e)=>{
        e.preventDefault();
      }
      switch(this.type){
      case "kick":
        this.setValue(1);
        this.sendEvent("change");
        break;
      case "toggle":
        if(e.ctrlKey || e.metaKey)
          this.value=defvalue;
        else
          this.value=1-this.value;
        this.checked=!!this.value;
        this.sendEvent("change");
        break;
      case "radio":
        let els=document.querySelectorAll("webaudio-switch[type='radio'][group='"+this.group+"']");
        for(let i=0;i<els.length;++i){
          if(els[i]==this)
            els[i].setValue(1);
          else
            els[i].setValue(0);
        }
        this.sendEvent("change");
        break;
      }

      window.addEventListener('mouseup', pointerup);
      window.addEventListener('touchend', pointerup);
      window.addEventListener('touchcancel', pointerup);
      document.body.addEventListener('touchstart', preventScroll,{passive:false});
      this.redraw();
      ev.preventDefault();
      ev.stopPropagation();
      return false;
    }
  });
} catch(error){
  console.log("webaudio-switch already defined");
}

try{
  customElements.define("webaudio-param", class WebAudioParam extends WebAudioControlsWidget {
    constructor(){
      super();
      this.addEventListener("keydown",this.keydown);
      this.addEventListener("mousedown",this.pointerdown,{passive:false});
      this.addEventListener("touchstart",this.pointerdown,{passive:false});
      this.addEventListener("wheel",this.wheel);
      this.addEventListener("mouseover",this.pointerover);
      this.addEventListener("mouseout",this.pointerout);
      this.addEventListener("contextmenu",this.contextMenu);
    }
    connectedCallback(){
      let root;
      if(this.attachShadow)
        root=this.attachShadow({mode: 'open'});
      else
        root=this;
      root.innerHTML=
`<style>
${this.basestyle}
:host{
  display:inline-block;
  user-select:none;
  margin:0;
  padding:0;
  font-family: sans-serif;
  font-size: 8px;
  cursor:pointer;
  position:relative;
  vertical-align:baseline;
}
.webaudio-param-body{
  display:inline-block;
  position:relative;
  text-align:center;
  background:none;
  margin:0;
  padding:0;
  font-family:sans-serif;
  font-size:11px;
  vertical-align:bottom;
  border:none;
}
</style>
<input class='webaudio-param-body' value='0' tabindex='1' touch-action='none'/><div class='webaudioctrl-tooltip'></div>
`;
      this.elem=root.childNodes[2];
      this.ttframe=root.childNodes[3];
      this.enable=this.getAttr("enable",1);
      this._value=this.getAttr("value",0); if (!this.hasOwnProperty("value")) Object.defineProperty(this,"value",{get:()=>{return this._value},set:(v)=>{this._value=v;this.redraw()}});
      this.defvalue=this.getAttr("defvalue",0);
      this._fontsize=this.getAttr("fontsize",9); if (!this.hasOwnProperty("fontsize")) Object.defineProperty(this,"fontsize",{get:()=>{return this._fontsize},set:(v)=>{this._fontsize=v;this.setupImage()}});
      this._src=this.getAttr("src",opt.paramSrc); if (!this.hasOwnProperty("src")) Object.defineProperty(this,"src",{get:()=>{return this._src},set:(v)=>{this._src=v;this.setupImage()}});
      this.link=this.getAttr("link","");
      this._width=this.getAttr("width",opt.paramWidth); if (!this.hasOwnProperty("width")) Object.defineProperty(this,"width",{get:()=>{return this._width},set:(v)=>{this._width=v;this.setupImage()}});
      this._height=this.getAttr("height",opt.paramHeight); if (!this.hasOwnProperty("height")) Object.defineProperty(this,"height",{get:()=>{return this._height},set:(v)=>{this._height=v;this.setupImage()}});
      this._colors=this.getAttr("colors",opt.paramColors); if (!this.hasOwnProperty("colors")) Object.defineProperty(this,"colors",{get:()=>{return this._colors},set:(v)=>{this._colors=v;this.setupImage()}});
      this.outline=this.getAttr("outline",opt.outline);
      this.rconv=this.getAttr("rconv",null);
      this.midiController={};
      this.midiMode="normal";
      this.currentLink=null;
      if(this.midicc) {
        let ch = parseInt(this.midicc.substring(0, this.midicc.lastIndexOf("."))) - 1;
        let cc = parseInt(this.midicc.substring(this.midicc.lastIndexOf(".") + 1));
        this.setMidiController(ch, cc);
      }
      this.setupImage();
      if(window.webAudioControlsWidgetManager)
//        window.webAudioControlsWidgetManager.updateWidgets();
        window.webAudioControlsWidgetManager.addWidget(this);
      this.fromLink=((e)=>{
        this.setValue(e.target.convValue.toFixed(e.target.digits));
      }).bind(this);
      this.elem.onchange=()=>{
        if(!this.currentLink.target.conv || (this.currentLink.target.conv&&this.rconv)){
          let val = this.value=this.elem.value;
          if(this.rconv){
            let x=+this.elem.value;
            val=eval(this.rconv);
          }
          if(this.currentLink){
            this.currentLink.target.setValue(val, true);
          }
        }
      }
    }
    disconnectedCallback(){}
    setupImage(){
      this.imgloaded=()=>{
        if(this.src!=""&&this.src!=null){
          this.elem.style.backgroundImage = "url("+this.src+")";
          this.elem.style.backgroundSize = "100% 100%";
          if(this._width==null) this._width=this.img.width;
          if(this._height==null) this._height=this.img.height;
        }
        else{
          if(this._width==null) this._width=32;
          if(this._height==null) this._height=20;
        }
        this.elem.style.width=this._width+"px";
        this.elem.style.height=this._height+"px";
        this.elem.style.fontSize=this.fontsize+"px";
        let l=document.getElementById(this.link);
        if(l&&typeof(l.value)!="undefined"){
          if(typeof(l.convValue)=="number")
            this.setValue(l.convValue.toFixed(l.digits));
          else
            this.setValue(l.convValue);
          if(this.currentLink)
            this.currentLink.removeEventListener("input",this.currentLink.func);
          this.currentLink={target:l, func:(e)=>{
            if(typeof(l.convValue)=="number")
              this.setValue(l.convValue.toFixed(l.digits));
            else
              this.setValue(l.convValue);
          }};
          this.currentLink.target.addEventListener("input",this.currentLink.func);
  //        l.addEventListener("input",(e)=>{this.setValue(l.convValue.toFixed(l.digits))});
        }
        this.redraw();
      };
      this.coltab = this.colors.split(";");
      this.elem.style.color=this.coltab[0];
      this.img=new Image();
      this.img.onload=this.imgloaded.bind();
      if(this.src==null){
        this.elem.style.backgroundColor=this.coltab[1];
        this.imgloaded();
      }
      else if(this.src==""){
        this.elem.style.background="none";
        this.imgloaded();
      }
      else{
        this.img.src=this.src;
      }
    }
    redraw() {
      this.elem.value=this.value;
    }
    setValue(v,f){
      this.value=v;
      if(this.value!=this.oldvalue){
        this.redraw();
        this.showtip(0);
        if(f){
          let event = new Event("change", {bubbles:opt.bubbles?true:false, cancelable:opt.cancelable?true:false});
          this.dispatchEvent(event);
        }
        this.oldvalue=this.value;
      }
    }
    pointerdown(ev){
      if(!this.enable)
        return;
      let e=ev;
      if(ev.touches)
          e = ev.touches[0];
      else {
        if(e.buttons!=1 && e.button!=0)
          return;
      }
      this.elem.focus();
      this.redraw();
    }
  });
} catch(error){
  console.log("webaudio-param already defined");
}

try{
  customElements.define("webaudio-keyboard", class WebAudioKeyboard extends WebAudioControlsWidget {
    constructor(){
      super();
    }
    connectedCallback(){
      let root;
      if(this.attachShadow)
        root=this.attachShadow({mode: 'open'});
      else
        root=this;
      root.innerHTML=
`<style>
${this.basestyle}
:host{
  display:inline-block;
  position:relative;
  margin:0;
  padding:0;
  font-family: sans-serif;
  font-size: 11px;
}
.webaudio-keyboard-body{
  display:inline-block;
  margin:0;
  padding:0;
  vertical-align:bottom;
}
</style>
<canvas class='webaudio-keyboard-body' tabindex='1' touch-action='none'></canvas><div class='webauioctrl-tooltip'></div>
`;
      this.elem=this.cv=root.childNodes[2];
      this.ttframe=root.childNodes[3];
      this.ctx=this.cv.getContext("2d");
      this._values=[];
      this.enable=this.getAttr("enable",1);
      this._width=this.getAttr("width",480); if (!this.hasOwnProperty("width")) Object.defineProperty(this,"width",{get:()=>{return this._width},set:(v)=>{this._width=v;this.setupImage()}});
      this._height=this.getAttr("height",128); if (!this.hasOwnProperty("height")) Object.defineProperty(this,"height",{get:()=>{return this._height},set:(v)=>{this._height=v;this.setupImage()}});
      this._min=this.getAttr("min",0); if (!this.hasOwnProperty("min")) Object.defineProperty(this,"min",{get:()=>{return this._min},set:(v)=>{this._min=+v;this.redraw()}});
      this._keys=this.getAttr("keys",25); if (!this.hasOwnProperty("keys")) Object.defineProperty(this,"keys",{get:()=>{return this._keys},set:(v)=>{this._keys=+v;this.setupImage()}});
      this._colors=this.getAttr("colors","#222;#eee;#ccc;#333;#000;#e88;#c44;#c33;#800"); if (!this.hasOwnProperty("colors")) Object.defineProperty(this,"colors",{get:()=>{return this._colors},set:(v)=>{this._colors=v;this.setupImage()}});
      this.outline=this.getAttr("outline",opt.outline);
      this.midilearn=this.getAttr("midilearn",0);
      this.midicc=this.getAttr("midicc",null);
      this.press=0;
      this.keycodes1=[90,83,88,68,67,86,71,66,72,78,74,77,188,76,190,187,191,226];
      this.keycodes2=[81,50,87,51,69,82,53,84,54,89,55,85,73,57,79,48,80,192,222,219];
      this.addEventListener("keyup",this.keyup);
      this.midiController={};
      this.midiMode="normal";
      if(this.midicc) {
          let ch = parseInt(this.midicc.substring(0, this.midicc.lastIndexOf("."))) - 1;
          let cc = parseInt(this.midicc.substring(this.midicc.lastIndexOf(".") + 1));
          this.setMidiController(ch, cc);
      }
      this.setupImage();
      this.digits=0;
      if(this.step && this.step < 1) {
        for(let n = this.step ; n < 1; n *= 10)
          ++this.digits;
      }
      if(window.webAudioControlsWidgetManager)
        window.webAudioControlsWidgetManager.addWidget(this);
    }
    disconnectedCallback(){}
    setupImage(){
      this.cv.style.width=this.width+"px";
      this.cv.style.height=this.height+"px";
      this.bheight = this.height * 0.55;
      this.kp=[0,7/12,1,3*7/12,2,3,6*7/12,4,8*7/12,5,10*7/12,6];
      this.kf=[0,1,0,1,0,0,1,0,1,0,1,0];
      this.ko=[0,0,(7*2)/12-1,0,(7*4)/12-2,(7*5)/12-3,0,(7*7)/12-4,0,(7*9)/12-5,0,(7*11)/12-6];
      this.kn=[0,2,4,5,7,9,11];
      this.coltab=this.colors.split(";");
      this.cv.width = this.width;
      this.cv.height = this.height;
      this.cv.style.width = this.width+'px';
      this.cv.style.height = this.height+'px';
      this.style.height = this.height+'px';
      this.cv.style.outline=this.outline?"":"none";
      this.bheight = this.height * 0.55;
      this.max=this.min+this.keys-1;
      this.dispvalues=[];
      this.disabledvalues=[];
      this.valuesold=[];
      if(this.kf[this.min%12])
        --this.min;
      if(this.kf[this.max%12])
        ++this.max;
      this.redraw();
    }
    redraw(){
      function rrect(ctx, x, y, w, h, r, c1, c2) {
        if(c2) {
          let g=ctx.createLinearGradient(x,y,x+w,y);
          g.addColorStop(0,c1);
          g.addColorStop(1,c2);
          ctx.fillStyle=g;
        }
        else
          ctx.fillStyle=c1;
        ctx.beginPath();
        ctx.moveTo(x, y);
        ctx.lineTo(x+w, y);
        ctx.lineTo(x+w, y+h-r);
        ctx.quadraticCurveTo(x+w, y+h, x+w-r, y+h);
        ctx.lineTo(x+r, y+h);
        ctx.quadraticCurveTo(x, y+h, x, y+h-r);
        ctx.lineTo(x, y);
        ctx.fill();
      }
      this.ctx.fillStyle = this.coltab[0];
      this.ctx.fillRect(0,0,this.width,this.height);
      let x0=7*((this.min/12)|0)+this.kp[this.min%12];
      let x1=7*((this.max/12)|0)+this.kp[this.max%12];
      let n=x1-x0;
      this.wwidth=(this.width-1)/(n+1);
      this.bwidth=this.wwidth*7/12;
      let h2=this.bheight;
      let r=Math.min(8,this.wwidth*0.2);
      for(let i=this.min,j=0;i<=this.max;++i) {
        if(this.kf[i%12]==0) {
          let x=this.wwidth*(j++)+1;
          if(this.disabledvalues.indexOf(i)>=0)
            rrect(this.ctx,x,1,this.wwidth-1,this.height-2,r,this.coltab[3],this.coltab[3]);
          else if(this.dispvalues.indexOf(i)>=0)
            rrect(this.ctx,x,1,this.wwidth-1,this.height-2,r,this.coltab[5],this.coltab[6]);
          else
            rrect(this.ctx,x,1,this.wwidth-1,this.height-2,r,this.coltab[1],this.coltab[2]);
        }
      }
      r=Math.min(8,this.bwidth*0.3);
      for(let i=this.min;i<this.max;++i) {
        if(this.kf[i%12]) {
          let x=this.wwidth*this.ko[this.min%12]+this.bwidth*(i-this.min)+1;
          if(this.disabledvalues.indexOf(i)>=0)
            rrect(this.ctx,x,1,this.bwidth,h2,r,this.coltab[3],this.coltab[3]);
          else if(this.dispvalues.indexOf(i)>=0)
            rrect(this.ctx,x,1,this.bwidth,h2,r,this.coltab[7],this.coltab[8]);
          else
            rrect(this.ctx,x,1,this.bwidth,h2,r,this.coltab[3],this.coltab[4]);
          this.ctx.strokeStyle=this.coltab[0];
          this.ctx.stroke();
        }
      }
    }
    _setValue(v){
      if(this.step)
        v=(Math.round((v-this.min)/this.step))*this.step+this.min;
      this._value=Math.min(this.max,Math.max(this.min,v));
      if(this._value!=this.oldvalue){
        this.oldvalue=this._value;
        this.redraw();
        this.showtip(0);
        return 1;
      }
      return 0;
    }
    setValue(v,f){
      if(this._setValue(v) && f)
        this.sendEvent("input"),this.sendEvent("change");
    }
    wheel(e){}
    keydown(e){
      let m=Math.floor((this.min+11)/12)*12;
      let k=this.keycodes1.indexOf(e.keyCode);
      if(k<0) {
        k=this.keycodes2.indexOf(e.keyCode);
        if(k>=0) k+=12;
      }
      if(k>=0){
        k+=m;
        if(this.currentKey!=k){
          this.currentKey=k;
          this.sendEventFromKey(1,k);
          this.setNote(1,k);
        }
      }
    }
    keyup(e){
      let m=Math.floor((this.min+11)/12)*12;
      let k=this.keycodes1.indexOf(e.keyCode);
      if(k<0) {
        k=this.keycodes2.indexOf(e.keyCode);
        if(k>=0) k+=12;
      }
      if(k>=0){
        k+=m;
        this.currentKey=-1;
        this.sendEventFromKey(0,k);
        this.setNote(0,k);
      }
    }
    pointerdown(ev){
      this.cv.focus();
      if(this.enable) {
        ++this.press;
      }
      let pointermove=(ev)=>{
        if(!this.enable)
          return;
        let r=this.getBoundingClientRect();
        let v=[],p;
        if(ev.touches)
          p=ev.targetTouches;
        else if(this.press)
          p=[ev];
        else
          p=[];
        if(p.length>0)
          this.drag=1;
        for(let i=0;i<p.length;++i) {
          let px=p[i].clientX-r.left;
          let py=p[i].clientY-r.top;
          let x,k,ko;
          if(py>=0&&py<this.height){
            if(py<this.bheight) {
              x=px-this.wwidth*this.ko[this.min%12];
              k=this.min+((x/this.bwidth)|0);
            }
            else {
              k=(px/this.wwidth)|0;
              ko=this.kp[this.min%12];
              k+=ko;
              k=this.min+((k/7)|0)*12+this.kn[k%7]-this.kn[ko%7];
            }
            if(k>=this.min&&k<=this.max)
              v.push(k);
          }
        }
        v.sort();
        this.values=v;
        this.sendevent();
        this.redraw();
      }
        
      let pointerup=(ev)=>{
        if(this.enable) {
          if(ev.touches)
            this.press=ev.touches.length;
          else
            this.press=0;
          pointermove(ev);
          this.sendevent();
          if(this.press==0){
            window.removeEventListener('mousemove', pointermove);
            window.removeEventListener('touchmove', pointermove, {passive:false});
            window.removeEventListener('mouseup', pointerup);
            window.removeEventListener('touchend', pointerup);
            window.removeEventListener('touchcancel', pointerup);
            document.body.removeEventListener('touchstart', preventScroll,{passive:false});
          }
          this.redraw();
        }
        this.drag=0;
        ev.preventDefault();
      }
      let preventScroll=(ev)=>{
        ev.preventDefault();
      }
      window.addEventListener('mousemove', pointermove);
      window.addEventListener('touchmove', pointermove, {passive:false});
      window.addEventListener('mouseup', pointerup);
      window.addEventListener('touchend', pointerup);
      window.addEventListener('touchcancel', pointerup);
      document.body.addEventListener('touchstart', preventScroll,{passive:false});
      pointermove(ev);
      ev.preventDefault();
      ev.stopPropagation();
    }
    sendEventFromKey(s,k){
      if (this.disabledvalues.includes(k)) return;
      let ev = new Event("change", {bubbles:opt.bubbles?true:false, cancelable:opt.cancelable?true:false});
      ev.note=[s,k];
      this.dispatchEvent(ev);
    }
    sendevent(){
      let notes=[];
      for(let i=0,j=this.valuesold.length;i<j;++i) {
        if(this.values.indexOf(this.valuesold[i])<0)
          notes.push([0,this.valuesold[i]]);
      }
      for(let i=0,j=this.values.length;i<j;++i) {
        if(this.valuesold.indexOf(this.values[i])<0)
          notes.push([1,this.values[i]]);
      }
      if(notes.length) {
        this.valuesold=this.values;
        for(let i=0;i<notes.length;++i) {
          if (this.disabledvalues.includes(notes[i][1])) return;
          this.setdispvalues(notes[i][0],notes[i][1]);
          let ev = new Event("change", {bubbles:opt.bubbles?true:false, cancelable:opt.cancelable?true:false});
          ev.note=notes[i];
          this.dispatchEvent(ev);
        }
      }
    }
    setDisabledRange(state,start,end) {
      for (var i=start; i<end;i++) {
        this.setdisabledvalues(state,i);
      }
      this.redraw();
    }
    setdisabledvalues(state,note) {
      let n=this.disabledvalues.indexOf(note);
      if(state) {
        if(n<0) this.disabledvalues.push(note);
      }
      else {
        if(n>=0) this.disabledvalues.splice(n,1);
      }
    }
    setdispvalues(state,note) {
      let n=this.dispvalues.indexOf(note);
      if(state) {
        if(n<0) this.dispvalues.push(note);
      }
      else {
        if(n>=0) this.dispvalues.splice(n,1);
      }
    }
    setNote(state,note,actx,when) {
      const t=(actx&&when-actx.currentTime);
      if(t>0){
        setTimeout(()=>{this.setNote(state,note)},t*1000);
      }
      else{
        this.setdispvalues(state,note);
        this.redraw();
      }
    }  });
} catch(error){
  console.log("webaudio-keyboard already defined");
}

  class WebAudioControlsWidgetManager {
    constructor(){
      this.midiAccess = null;
      this.listOfWidgets = [];
      this.listOfExternalMidiListeners = [];
      this.updateWidgets();
      if(opt.preserveMidiLearn)
        this.midiLearnTable=JSON.parse(localStorage.getItem("WebAudioControlsMidiLearn"));
      else
        this.midiLearnTable=null;
      this.initWebAudioControls();
    }
    addWidget(w){
      this.listOfWidgets.push(w);
    }
    updateWidgets(){
//      this.listOfWidgets = document.querySelectorAll("webaudio-knob,webaudio-slider,webaudio-switch");
    }
    initWebAudioControls() {
      if(navigator.requestMIDIAccess) {
        navigator.requestMIDIAccess().then(
          (ma)=>{this.midiAccess = ma,this.enableInputs()},
          (err)=>{ console.log("MIDI not initialized - error encountered:" + err.code)}
        );
      }
    }
    enableInputs() {
      let inputs = this.midiAccess.inputs.values();
      console.log("Found " + this.midiAccess.inputs.size + " MIDI input(s)");
      for(let input = inputs.next(); input && !input.done; input = inputs.next()) {
        console.log("Connected input: " + input.value.name);
        input.value.onmidimessage = this.handleMIDIMessage.bind(this);
      }
    }
    midiConnectionStateChange(e) {
      console.log("connection: " + e.port.name + " " + e.port.connection + " " + e.port.state);
      enableInputs();
    }

    onMIDIStarted(midi) {
      this.midiAccess = midi;
      midi.onstatechange = this.midiConnectionStateChange;
      enableInputs(midi);
    }
    // Add hooks for external midi listeners support
    addMidiListener(callback) {
      this.listOfExternalMidiListeners.push(callback);
    }
    getCurrentConfigAsJSON() {
      return currentConfig.stringify();
    }
    handleMIDIMessage(event) {
      this.listOfExternalMidiListeners.forEach(function (externalListener) {
        externalListener(event);
      });
      if(((event.data[0] & 0xf0) == 0xf0) || ((event.data[0] & 0xf0) == 0xb0 && event.data[1] >= 120))
        return;
      for(let w of this.listOfWidgets) {
        if(w.processMidiEvent)
          w.processMidiEvent(event);
      }
      if(opt.mididump)
        console.log(event.data);
    }
    contextMenuOpen(e,knob){
      if(!this.midiAccess)
        return;
      let menu=document.getElementById("webaudioctrl-context-menu");
      menu.style.left=e.pageX+"px";
      menu.style.top=e.pageY+"px";
      menu.knob=knob;
      menu.classList.add("active");
      menu.knob.focus();
      menu.knob.addEventListener("keydown",this.contextMenuCloseByKey.bind(this));
    }
    contextMenuCloseByKey(e){
      if(e.keyCode==27)
       this.contextMenuClose();
    }
    contextMenuClose(){
      let menu=document.getElementById("webaudioctrl-context-menu");
      menu.knob.removeEventListener("keydown",this.contextMenuCloseByKey);
      menu.classList.remove("active");
      let menuItemLearn=document.getElementById("webaudioctrl-context-menu-learn");
      menuItemLearn.innerHTML = 'Learn';
      menu.knob.midiMode = 'normal';
    }
    contextMenuLearn(){
      let menu=document.getElementById("webaudioctrl-context-menu");
      let menuItemLearn=document.getElementById("webaudioctrl-context-menu-learn");
      menuItemLearn.innerHTML = 'Listening...';
      menu.knob.midiMode = 'learn';
    }
    contextMenuClear(e){
      let menu=document.getElementById("webaudioctrl-context-menu");
      menu.knob.midiController={};
      this.contextMenuClose();
    }
    preserveMidiLearn(){
      if(!opt.preserveMidiLearn)
        return;
      const v=[];
      for(let w of this.listOfWidgets) {
        if(w.id)
          v.push({"id":w.id, "cc":w.midiController});
      }
      const s=JSON.stringify(v);
      localStorage.setItem("WebAudioControlsMidiLearn",s);
    }
  }
  if(window.UseWebAudioControlsMidi||opt.useMidi)
    window.webAudioControlsWidgetManager = window.webAudioControlsMidiManager = new WebAudioControlsWidgetManager();
}

// ── shared.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Shared Utilities
// Vanilla JS · Shoelace Web Components
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

// ─── Constants ───────────────────────────────────────────────
const API_V2 = '/api/v2';

// sun-fill SVG not in our Shoelace bundle — register it as a data URI
// so the theme toggle doesn't trigger a network fetch
const SUN_FILL_SVG = '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16"><path d="M8 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8M8 0a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 0m0 13a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 13m8-5a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2a.5.5 0 0 1 .5.5M3 8a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2A.5.5 0 0 1 3 8m10.657-5.657a.5.5 0 0 1 0 .707l-1.414 1.415a.5.5 0 1 1-.707-.708l1.414-1.414a.5.5 0 0 1 .707 0m-9.193 9.193a.5.5 0 0 1 0 .707L3.05 13.657a.5.5 0 0 1-.707-.707l1.414-1.414a.5.5 0 0 1 .707 0m9.193 2.121a.5.5 0 0 1-.707 0l-1.414-1.414a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707M3.757 4.464a.5.5 0 0 1-.707 0L1.636 3.05a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707"/></svg>';

// ═══════════════════════════════════════════════════════════════
//  HTML ESCAPING
// ═══════════════════════════════════════════════════════════════

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}

// ═══════════════════════════════════════════════════════════════
//  FORMATTING
// ═══════════════════════════════════════════════════════════════

function formatBytes(bytes) {
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
  if (bytes < 1073741824) return `${(bytes / 1048576).toFixed(1)} MB`;
  return `${(bytes / 1073741824).toFixed(1)} GB`;
}

// ═══════════════════════════════════════════════════════════════
//  API CLIENT
// ═══════════════════════════════════════════════════════════════

/**
 * GET request to /api/v2/<path>
 * @param {string} path - e.g. '/plugins?action=list'
 * @returns {Promise<any>} parsed JSON response
 */
// Default timeout for API read requests (ms).
// ESP32 responses are fast (<200ms) but USB NCM can stall;
// 10s matches server-side recv/send_wait_timeout.
var API_TIMEOUT_MS = 10000;
// Longer timeout for mutation operations (loadPreset, savePreset, etc.)
// that trigger SD card I/O on the firmware side.
var API_MUTATION_TIMEOUT_MS = 20000;
// Extra-long timeout for plugin switches — WTOsc, WTOscDuo, Freakwaves, VctrSnt
// trigger ctagSampleRom SD card loading (all wavetable/sample data into PSRAM),
// which can take 15-30+ seconds. The firmware blocks the HTTP response until done.
var API_PLUGIN_SWITCH_TIMEOUT_MS = 45000;

// ─── Circuit Breaker ─────────────────────────────────────────
// Track consecutive API failures.  After _FAILURE_THRESHOLD in a row,
// trigger disconnect + drain the request queue to stop hammering a
// dead device.  Threshold is generous because the ESP32 httpd can
// be temporarily unresponsive during heavy plugin allocation
// (mutex contention with audio task, SD card I/O).
var _consecutiveFailures = 0;
var _FAILURE_THRESHOLD = 4;

async function apiFetch(path, timeoutMs, skipCircuitBreaker) {
  timeoutMs = timeoutMs || API_TIMEOUT_MS;
  try {
    const r = await fetch(`${API_V2}${path}`, {
      signal: AbortSignal.timeout(timeoutMs),
    });
    if (!r.ok) throw new Error(`API ${r.status}`);
    _consecutiveFailures = 0;
    var text = await r.text();
    if (!text || !text.trim()) return {};
    try { return JSON.parse(text); } catch(e) { return {}; }
  } catch(e) {
    // Don't count toward circuit breaker if caller opted out.
    // Plugin switch timeouts mean the device is busy (loading sample ROM),
    // NOT offline.  Only genuine network errors should trigger disconnect.
    if (!skipCircuitBreaker) {
      _consecutiveFailures++;
      if (_consecutiveFailures >= _FAILURE_THRESHOLD) {
        setDisconnected();
        apiQueue.drain();
      }
    }
    throw e;
  }
}

/**
 * POST request to /api/v2/<path> with optional JSON body.
 * If body is null/undefined, sends a POST with no body (for query-param-only mutations).
 * @param {string} path - e.g. '/device?action=setConfig'
 * @param {object|null} body - JSON-serializable data, or null for body-less POST
 * @returns {Promise<any>} parsed JSON response
 */
async function apiPostJSON(path, body, timeoutMs, skipCircuitBreaker) {
  timeoutMs = timeoutMs || API_TIMEOUT_MS;
  try {
    var opts = {
      method: 'POST',
      signal: AbortSignal.timeout(timeoutMs),
    };
    if (body != null) {
      opts.headers = { 'Content-Type': 'application/json' };
      opts.body = JSON.stringify(body);
    }
    const r = await fetch(`${API_V2}${path}`, opts);
    if (!r.ok) throw new Error(`API ${r.status}`);
    _consecutiveFailures = 0;
    var text = await r.text();
    if (!text || !text.trim()) return {};
    try { return JSON.parse(text); } catch(e) { return {}; }
  } catch(e) {
    if (!skipCircuitBreaker) {
      _consecutiveFailures++;
      if (_consecutiveFailures >= _FAILURE_THRESHOLD) {
        setDisconnected();
        apiQueue.drain();
      }
    }
    throw e;
  }
}

// ═══════════════════════════════════════════════════════════════
//  FETCH QUEUE — Serialize requests to avoid overwhelming ESP32
// ═══════════════════════════════════════════════════════════════

class FetchQueue {
  constructor() {
    this._queue = [];
    this._running = false;
    this._paused = false;
  }

  enqueue(fn) {
    if (this._paused) {
      return Promise.reject(new Error('Device offline \u2014 request cancelled'));
    }
    return new Promise((resolve, reject) => {
      this._queue.push({ fn, resolve, reject });
      this._process();
    });
  }

  async _process() {
    if (this._running) return;
    this._running = true;
    while (this._queue.length && !this._paused) {
      const { fn, resolve, reject } = this._queue.shift();
      try { resolve(await fn()); } catch (e) { reject(e); }
    }
    this._running = false;
  }

  /** Reject all pending items and pause the queue. */
  drain() {
    this._paused = true;
    while (this._queue.length) {
      var item = this._queue.shift();
      item.reject(new Error('Device offline \u2014 request cancelled'));
    }
  }

  /** Resume accepting new items after reconnection. */
  resume() {
    this._paused = false;
  }
}

// Shared queue for serializing parameter SET calls across views
const paramQueue = new FetchQueue();

// Global API queue — ALL API calls route through this to keep
// max 1 in-flight request at a time (ESP32 has only ~4 usable sockets).
const apiQueue = new FetchQueue();

/**
 * Queue-wrapped apiFetch — serializes all GET requests.
 * @param {string} path - e.g. '/plugins?action=list'
 * @returns {Promise<any>} parsed JSON response
 */
function queuedFetch(path, timeoutMs, skipCircuitBreaker) {
  return apiQueue.enqueue(function() { return apiFetch(path, timeoutMs, skipCircuitBreaker); });
}

/**
 * Queue-wrapped apiPostJSON — serializes all POST requests.
 * @param {string} path - e.g. '/device?action=setConfig'
 * @param {object|null} body
 * @param {number} [timeoutMs]
 * @param {boolean} [skipCircuitBreaker]
 * @returns {Promise<any>} parsed JSON response
 */
function queuedPost(path, body, timeoutMs, skipCircuitBreaker) {
  return apiQueue.enqueue(function() { return apiPostJSON(path, body, timeoutMs, skipCircuitBreaker); });
}

// ═══════════════════════════════════════════════════════════════
//  TOAST NOTIFICATIONS
// ═══════════════════════════════════════════════════════════════

var _toastBusy = false;
var _toastCount = 0;           // throttle: max toasts in flight
var _TOAST_MAX_PENDING = 5;    // discard beyond this to avoid DOM leak

function toast(message, variant, duration) {
  if (_toastBusy) return;          // prevent synchronous recursion
  if (_toastCount >= _TOAST_MAX_PENDING) return;  // throttle
  _toastBusy = true;
  _toastCount++;
  try {
    variant = variant || 'primary';
    duration = duration || 4000;
    var stack = document.getElementById('toast-stack');
    if (!stack) { _toastBusy = false; _toastCount--; return; }
    var alert = document.createElement('sl-alert');
    alert.variant = variant;
    alert.closable = true;
    alert.duration = duration;
    alert.innerHTML = '<sl-icon slot="icon" name="' + iconForVariant(variant) + '">' + '</sl-icon>' + esc(message);
    // Decrement counter when alert is removed from DOM (auto-hide or close)
    alert.addEventListener('sl-after-hide', function() { _toastCount = Math.max(0, _toastCount - 1); });
    stack.appendChild(alert);
    // Only call .toast() if sl-alert is already defined; skip silently otherwise
    if (customElements.get('sl-alert')) {
      try { alert.toast(); } catch(e) { console.warn('toast render failed:', e); }
    }
    // If sl-alert not defined yet, just leave it in DOM (don't queue .whenDefined
    // which causes async re-entrancy problems)
  } catch(e) {
    console.warn('toast() error:', e);
    _toastCount = Math.max(0, _toastCount - 1);
  } finally {
    _toastBusy = false;
  }
}

function iconForVariant(v) {
  var map = {
    success: 'check2-circle',
    warning: 'exclamation-triangle',
    danger: 'exclamation-octagon',
    primary: 'info-circle',
    neutral: 'info-circle'
  };
  return map[v] || 'info-circle';
}

// ═══════════════════════════════════════════════════════════════
//  THEME MANAGEMENT
// ═══════════════════════════════════════════════════════════════

function setupThemeToggle(btnId) {
  var btn = document.getElementById(btnId || 'theme-toggle');
  if (!btn) return;
  var saved = localStorage.getItem('tbd-theme');
  if (saved === 'light') applyTheme('light');
  btn.addEventListener('click', function() {
    var isDark = document.documentElement.classList.contains('sl-theme-dark');
    applyTheme(isDark ? 'light' : 'dark');
  });
}

function applyTheme(theme) {
  var html = document.documentElement;
  var btn  = document.getElementById('theme-toggle');
  if (theme === 'light') {
    html.classList.remove('sl-theme-dark');
    html.classList.add('sl-theme-light');
    if (btn) btn.name = 'sun-fill';
  } else {
    html.classList.remove('sl-theme-light');
    html.classList.add('sl-theme-dark');
    if (btn) btn.name = 'moon-fill';
  }
  // If page loads only one theme <link> (e.g. index.html), swap its href
  var links = document.querySelectorAll('link[href*="/shoelace/themes/"]');
  if (links.length === 1) {
    links[0].href = '/shoelace/themes/' + theme + '.css?v=3';
  }
  // If both themes are pre-loaded (e.g. preset-macro-manager.html), class toggle suffices
  localStorage.setItem('tbd-theme', theme);
}

// ═══════════════════════════════════════════════════════════════
//  CONNECTION MONITOR
// ═══════════════════════════════════════════════════════════════

var connectionState = {
  status: 'connecting',   // 'connecting' | 'connected' | 'disconnected'
  retries: 0,
  maxRetries: 60,
  pollIntervalMs: 5000,   // WLED uses 5s for HTTP fallback reconnect
  _timer: null,
  _onConnect: null,
  _onDisconnect: null,
};

function startConnectionMonitor(onConnect, onDisconnect) {
  connectionState._onConnect = onConnect;
  connectionState._onDisconnect = onDisconnect;
}

function setConnected() {
  if (connectionState.status === 'connected') return;
  connectionState.status = 'connected';
  connectionState.retries = 0;
  _consecutiveFailures = 0;
  apiQueue.resume();
  updateConnectionUI();
  if (connectionState._onConnect) connectionState._onConnect();
}

function setDisconnected() {
  if (connectionState.status === 'disconnected') return;
  connectionState.status = 'disconnected';
  updateConnectionUI();
  if (connectionState._onDisconnect) connectionState._onDisconnect();
  scheduleReconnect();
}

function scheduleReconnect() {
  if (connectionState._timer) return;
  connectionState._timer = setInterval(async function() {
    if (connectionState.retries >= connectionState.maxRetries) {
      clearInterval(connectionState._timer);
      connectionState._timer = null;
      return;
    }
    // Skip poll if the API queue is busy (user-initiated request in-flight)
    if (apiQueue._running) return;
    connectionState.retries++;
    try {
      await apiFetch('/device?action=getIOCaps');
      clearInterval(connectionState._timer);
      connectionState._timer = null;
      setConnected();
    } catch (e) {
      // still disconnected
    }
  }, connectionState.pollIntervalMs);
}

function updateConnectionUI() {
  var el = document.getElementById('status-text');
  if (el) {
    if (connectionState.status === 'connected') {
      el.textContent = 'Connected';
      el.style.color = 'var(--sl-color-success-600)';
    } else if (connectionState.status === 'disconnected') {
      el.textContent = 'Offline';
      el.style.color = 'var(--sl-color-danger-600)';
    } else {
      el.textContent = 'Connecting\u2026';
      el.style.color = 'var(--sl-color-neutral-500)';
    }
  }
  // Update footer connection dot
  var dot = document.getElementById('footer-conn-dot');
  var txt = document.getElementById('footer-conn-text');
  if (dot) {
    dot.classList.toggle('offline', connectionState.status !== 'connected');
  }
  if (txt) {
    txt.textContent = connectionState.status === 'connected' ? 'Connected' :
                      connectionState.status === 'disconnected' ? 'Offline' : 'Connecting\u2026';
  }
  // Update header connection pill
  var pill = document.getElementById('conn-pill');
  var pillText = document.getElementById('conn-pill-text');
  if (pill) {
    pill.classList.toggle('offline', connectionState.status !== 'connected');
  }
  if (pillText) {
    pillText.textContent = connectionState.status === 'connected' ? 'Connected' :
                           connectionState.status === 'disconnected' ? 'Offline' : 'Connecting\u2026';
  }
}

// ═══════════════════════════════════════════════════════════════
//  DIALOG HELPERS
// ═══════════════════════════════════════════════════════════════

/**
 * Wrap a Shoelace sl-dialog in a Promise for await-friendly confirmation.
 * @param {string} dialogId - DOM id of the sl-dialog
 * @param {string} okBtnId - DOM id of the confirm button
 * @param {string} cancelBtnId - DOM id of the cancel button
 * @returns {Promise<boolean>} true if confirmed, false if cancelled
 */
function confirmDialog(dialogId, okBtnId, cancelBtnId) {
  return new Promise(function(resolve) {
    var dlg = document.getElementById(dialogId);
    var ok  = document.getElementById(okBtnId);
    var cancel = document.getElementById(cancelBtnId);
    if (!dlg) { resolve(false); return; }

    function cleanup() {
      ok.removeEventListener('click', onOk);
      if (cancel) cancel.removeEventListener('click', onCancel);
      dlg.removeEventListener('sl-request-close', onCancel);
    }
    function onOk() { cleanup(); dlg.hide(); resolve(true); }
    function onCancel() { cleanup(); dlg.hide(); resolve(false); }

    ok.addEventListener('click', onOk);
    if (cancel) cancel.addEventListener('click', onCancel);
    dlg.addEventListener('sl-request-close', onCancel);
    dlg.show();
  });
}

// ═══════════════════════════════════════════════════════════════
//  LOADING OVERLAY — visual feedback during heavy operations
// ═══════════════════════════════════════════════════════════════

function showLoading(message) {
  var overlay = document.getElementById('loading-overlay');
  var text = document.getElementById('loading-text');
  if (overlay) {
    if (text) text.textContent = message || 'Loading\u2026';
    overlay.classList.remove('hidden');
  }
}

function hideLoading() {
  var overlay = document.getElementById('loading-overlay');
  if (overlay) overlay.classList.add('hidden');
}

// ═══════════════════════════════════════════════════════════════
//  EXPORTS — attach to window for non-module scripts
// ═══════════════════════════════════════════════════════════════

// ─── Control Mode ──────────────────────────────────────────
var _waLoaded = false;

function isControlMode() {
  return localStorage.getItem('tbd-control-mode') === '1';
}

function setControlMode(on) {
  localStorage.setItem('tbd-control-mode', on ? '1' : '0');
  if (on && !_waLoaded) loadWebAudioControls();
}

function loadWebAudioControls() {
  // webaudio-controls.js is now included in the app-bundle.js
  // so it's always available — no dynamic loading needed
  _waLoaded = true;
  return Promise.resolve();
}

// ═══════════════════════════════════════════════════════════════
//  SVG KNOB RENDERER — matching webaudio-controls style
// ═══════════════════════════════════════════════════════════════

/**
 * Render an SVG rotary knob matching the webaudio-controls style.
 *
 * opts.value   – current value (default 0)
 * opts.min     – minimum value (default 0)
 * opts.max     – maximum value (default 127)
 * opts.size    – pixel diameter (default 52)
 * opts.color   – 'normal' (dark charcoal) | 'macro' (orange/gold)
 *
 * A "macro knob" controls 2+ DSP parameters via the mapping formula.
 */
function renderKnobSVG(opts) {
  var value = opts.value || 0;
  var min = opts.min || 0;
  var max = opts.max || 127;
  var size = opts.size || 52;
  var color = opts.color || 'normal';

  var pct = max > min ? ((value - min) / (max - min)) : 0;
  pct = Math.max(0, Math.min(1, pct));

  var cx = size / 2;
  var cy = size / 2;
  var r = (size / 2) - 2;

  // Rotation: 270° sweep from -135° to +135° (bottom-left to bottom-right)
  var angle = -135 + pct * 270;
  var rad = angle * Math.PI / 180;

  // Indicator line: from ~40% radius to ~88% radius
  var x1 = cx + r * 0.40 * Math.sin(rad);
  var y1 = cy - r * 0.40 * Math.cos(rad);
  var x2 = cx + r * 0.88 * Math.sin(rad);
  var y2 = cy - r * 0.88 * Math.cos(rad);

  // Color schemes — matching webaudio-controls colors attribute
  // colors = "indicator ; outerFill ; centerFill"
  var indicator, outerFill, centerFill, outerStroke;
  if (color === 'macro') {
    // Orange/gold for macro knobs (controls 2+ DSP params)
    indicator  = '#fef3c7';  // warm light yellow
    outerFill  = '#92400e';  // amber-800
    centerFill = '#b45309';  // amber-700
    outerStroke = '#78350f'; // amber-900
  } else if (color === 'mix') {
    // Light/silver knobs for the inverted Mix page
    indicator  = '#334155';  // dark slate indicator
    outerFill  = '#cbd5e1';  // slate-300
    centerFill = '#e2e8f0';  // slate-200
    outerStroke = '#94a3b8'; // slate-400
  } else {
    // Dark charcoal for normal knobs (1:1 mapping)
    indicator  = '#ccc';
    outerFill  = '#484848';
    centerFill = '#525252';
    outerStroke = '#3a3a3a';
  }

  // Use unique gradient IDs to avoid conflicts when multiple knobs are rendered
  var uid = 'k' + Math.random().toString(36).substr(2, 5);

  var svg = '';
  svg += '<svg width="' + size + '" height="' + size + '" viewBox="0 0 ' + size + ' ' + size + '" class="knob-svg">';

  // Definitions for gradients
  svg += '<defs>';
  // Radial gradient: center lighter, edge darker
  svg += '<radialGradient id="' + uid + 'g" cx="50%" cy="50%">';
  svg += '<stop offset="0%" stop-color="' + centerFill + '"/>';
  svg += '<stop offset="100%" stop-color="' + outerFill + '"/>';
  svg += '</radialGradient>';
  // Subtle bottom shadow
  svg += '<linearGradient id="' + uid + 's" x1="0" y1="0" x2="0" y2="1">';
  svg += '<stop offset="0%" stop-color="#000" stop-opacity="0"/>';
  svg += '<stop offset="100%" stop-color="#000" stop-opacity="0.15"/>';
  svg += '</linearGradient>';
  svg += '</defs>';

  // Outer shadow halo
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + r + '" fill="' + outerFill + '" opacity="0.2"/>';

  // Main knob body with gradient
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + (r - 1) + '" fill="url(#' + uid + 'g)"/>';

  // Bottom shadow overlay
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + (r - 1) + '" fill="url(#' + uid + 's)"/>';

  // Edge ring
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + (r - 1) + '" fill="none" stroke="' + outerStroke + '" stroke-width="0.5"/>';

  // Indicator tick line
  svg += '<line x1="' + x1.toFixed(1) + '" y1="' + y1.toFixed(1) + '" x2="' + x2.toFixed(1) + '" y2="' + y2.toFixed(1) + '" ';
  svg += 'stroke="' + indicator + '" stroke-width="2.5" stroke-linecap="butt"/>';

  svg += '</svg>';
  return svg;
}

/**
 * Analyze a macro definition's mappings to determine which virtual knob
 * indices are "macro" (control 2+ DSP parameters).
 * Returns: { paramIdx: [{ ctrl, start, mul, div }, ...], ... }
 */
function analyzeMappings(def) {
  var result = {};
  if (!def || !def.mapping) return result;

  def.mapping.forEach(function(m) {
    if (!m.add) return;
    m.add.forEach(function(a) {
      if (!result[a.src]) result[a.src] = [];
      result[a.src].push({ ctrl: m.ctrl, start: m.start || 0, mul: a.mul, div: a.div });
    });
  });
  return result;
}

/**
 * Check if a virtual parameter is a "macro knob" (controls 2+ DSP params).
 */
function isMacroKnob(mappingAnalysis, paramIdx) {
  var entries = mappingAnalysis[paramIdx];
  return entries && entries.length >= 2;
}

/**
/**
 * Apply a response curve to a 0-127 value.
 * Must match the C++ applyCurve() in MacroTranslator.cpp exactly.
 */
function applyCurve(val, curveType) {
  if (!curveType || curveType === 'linear') return val;
  if (val <= 0) return 0;
  if (val >= 127) return 127;

  switch (curveType) {
    case 'log':
      if (val <= 16) return val * 4;
      if (val <= 64) return 64 + Math.round((val - 16) * 36 / 48);
      return 100 + Math.round((val - 64) * 27 / 63);

    case 'exp':
      return Math.round(val * val / 127);

    default:
      return val;
  }
}

/**
 * Compute the real CC output values for a given knob value.
 * Returns an array of { ctrl, name, value, pct } for each mapping target.
 *   ctrl  — CC number
 *   name  — human-readable DSP param name
 *   value — computed output (0-127)
 *   pct   — percentage of 127 (for bar display)
 */
function computeMappingOutputs(def, paramIdx, knobValue) {
  if (!def || !def.mapping) return [];
  var results = [];
  def.mapping.forEach(function(m) {
    if (!m.add) return;
    m.add.forEach(function(a) {
      if (a.src !== paramIdx) return;
      var curved = applyCurve(knobValue, a.curve);
      var val = (m.start || 0) + Math.round(curved * a.mul / a.div);
      val = Math.max(0, Math.min(127, val));
      results.push({
        ctrl: m.ctrl,
        name: resolveCCName(def.machine, m.ctrl),
        value: val,
        pct: Math.round(val / 127 * 100)
      });
    });
  });
  return results;
}

/**
 * Resolve a CC number to the human-readable parameter name for a given machine.
 * Returns the parameter name (e.g. "Freq") or "CC <n>" if not found.
 */
function resolveCCName(machineId, ctrl) {
  var info = getMachineInfo(machineId);
  if (!info || !info.parameters) return 'CC ' + ctrl;
  var param = info.parameters.find(function(p) { return p.ctrl === ctrl; });
  return param ? param.name : 'CC ' + ctrl;
}

// ═══════════════════════════════════════════════════════════════
//  SHARED DATA STORE — both Performer and Designer use this
// ═══════════════════════════════════════════════════════════════

var sharedData = {
  synthDefs: null,
  tracks: [],
  machines: [],
  macroDefs: [],
  soundPresets: [],
  activeTrack: -1,
  loaded: false,
};

var _trackChangeCallbacks = [];

/**
 * Load all data from the device (synthdefs, macrodefs, soundpresets, tracks).
 * Called once at boot; both views read from sharedData.
 *
 * Uses TWO sequential requests to stay within ESP32 HTTP socket limits:
 *   1. GET /api/v2/samples?getconfig=synthdefinitions.json  → synth defs
 *   2. GET /api/v2/macros?action=getall                     → bulk macro data
 *
 * The "getall" endpoint returns { macroDefs, soundPresets, tracks } in a
 * single response, replacing the previous 60+ individual file-fetches.
 */
function loadSharedData() {
  showLoading('Loading tracks & definitions…');
  return fetch('/api/v2/samples?getconfig=synthdefinitions.json').then(function(r) {
    if (!r.ok) throw new Error('HTTP ' + r.status);
    return r.json();
  }).then(function(synthDefs) {
    sharedData.synthDefs = synthDefs;
    sharedData.tracks = synthDefs.tracks || [];
    sharedData.machines = synthDefs.machines || [];

    return fetch('/api/v2/macros?action=getall').then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }).then(function(macroData) {
    sharedData.macroDefs = macroData.macroDefs || [];
    sharedData.soundPresets = macroData.soundPresets || [];
    // Merge firmware track state into rich synthDefs tracks (don't overwrite!)
    if (macroData.tracks && Array.isArray(macroData.tracks)) {
      macroData.tracks.forEach(function(fwTrack) {
        var existing = sharedData.tracks.find(function(t) { return t.index === fwTrack.index; });
        if (existing) {
          if (fwTrack.machine) existing.currentMachine = fwTrack.machine;
          if (fwTrack.macro) existing.currentMacro = fwTrack.macro;
        }
      });
    }
    sharedData.loaded = true;
    setConnected();
    hideLoading();
    console.log('[Shared] Loaded:', sharedData.tracks.length, 'tracks,',
                sharedData.machines.length, 'machines,',
                sharedData.macroDefs.length, 'macro defs,',
                sharedData.soundPresets.length, 'sound presets');
    return sharedData;
  }).catch(function(err) {
    hideLoading();
    console.error('[Shared] Load error:', err);
    toast('Failed to load data: ' + err.message, 'danger', 4000);
    throw err;
  });
}

/**
 * Reload macro definitions and sound presets from device (after save/delete).
 * Single request via the bulk macroapi endpoint.
 */
function reloadMacroData() {
  return fetch('/api/v2/macros?action=getall').then(function(r) {
    return r.ok ? r.json() : { macroDefs: [], soundPresets: [] };
  }).then(function(macroData) {
    sharedData.macroDefs = macroData.macroDefs || [];
    sharedData.soundPresets = macroData.soundPresets || [];
    // Merge firmware track state into rich synthDefs tracks (don't overwrite!)
    if (macroData.tracks && Array.isArray(macroData.tracks)) {
      macroData.tracks.forEach(function(fwTrack) {
        var existing = sharedData.tracks.find(function(t) { return t.index === fwTrack.index; });
        if (existing) {
          if (fwTrack.machine) existing.currentMachine = fwTrack.machine;
          if (fwTrack.macro) existing.currentMacro = fwTrack.macro;
        }
      });
    }
    return sharedData;
  });
}

/**
 * Tell firmware to reload macros from disk (after saving/deleting definitions).
 * Disables processing, calls RefreshMacros(), re-enables processing.
 */
function reloadFirmwareMacros() {
  return fetch('/api/v2/macros?action=reload', { method: 'POST' })
    .then(function(r) { return r.ok ? r.json() : null; })
    .catch(function(err) {
      console.warn('[Shared] Firmware macro reload failed:', err);
    });
}

/**
 * Register a callback for track changes.
 * Callback receives (trackIndex, track).
 */
function onTrackChange(callback) {
  _trackChangeCallbacks.push(callback);
}

/**
 * Select a track. Updates shared state and notifies all listeners.
 */
function selectSharedTrack(idx) {
  var track = sharedData.tracks.find(function(t) { return t.index === idx; });
  if (!track) return;

  sharedData.activeTrack = idx;

  // Update track strip visuals
  document.querySelectorAll('.track-strip').forEach(function(s) {
    s.classList.toggle('active', parseInt(s.getAttribute('data-track'), 10) === idx);
  });

  // Notify all registered listeners
  _trackChangeCallbacks.forEach(function(cb) {
    try { cb(idx, track); } catch(e) { console.error('Track change callback error:', e); }
  });
}

/**
 * Get machine info by id from shared data.
 */
function getMachineInfo(machineId) {
  return sharedData.machines.find(function(m) { return m.id === machineId; }) || null;
}

/**
 * Get available (non-empty) machines for a track.
 */
function getTrackMachines(track) {
  return (track.machines || []).filter(function(m) {
    return m !== 'nodrum' && m !== 'nosynth' && m !== 'nofx';
  });
}

/**
 * Render the shared track overview strip.
 */
function renderTrackOverview() {
  var container = document.getElementById('track-overview');
  if (!container) return;

  var html = '';
  sharedData.tracks.forEach(function(track) {
    var classes = 'track-strip';
    if (track.index >= 16 && track.index <= 17) classes += ' track-fx';
    if (track.index === 18) classes += ' track-master';
    if (track.index === sharedData.activeTrack) classes += ' active';

    var avail = getTrackMachines(track);
    var defaultMachine = avail.length > 0 ? avail[0] : '—';

    html += '<div class="' + classes + '" data-track="' + track.index + '">';
    html += '<span class="track-num">' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-name">' + esc(track.name) + '</span>';
    html += '</div>';
  });

  container.innerHTML = html;
}

/**
 * Set up click events on the shared track overview strip.
 */
function setupTrackOverviewEvents() {
  var container = document.getElementById('track-overview');
  if (!container) return;

  container.addEventListener('click', function(e) {
    var strip = e.target.closest('.track-strip');
    if (!strip) return;
    var trackIdx = parseInt(strip.getAttribute('data-track'), 10);
    selectSharedTrack(trackIdx);
  });
}

// ═══════════════════════════════════════════════════════════════
// Shared Knob Group Renderer
// Renders macro definition knob groups identically for both
// the Presets view (interactive) and the Macros view (preview).
//
// Parameters:
//   def         — macro definition object (groups, mapping, machine)
//   paramValues — array of current knob values (by param.idx)
//                 if null, uses param.def defaults
//   options     — { knobSize: 64 }
//
// Returns an HTML string (no container div — caller wraps).
// ═══════════════════════════════════════════════════════════════
function renderKnobGroups(def, paramValues, options) {
  if (!def || !def.groups) return '';
  var opts = options || {};
  var knobSize = opts.knobSize || 64;
  var mappingInfo = analyzeMappings(def);
  var html = '';
  var hasParams = false;
  var visiblePageNum = 0;  // count only groups that have parameters

  def.groups.forEach(function(group, gi) {
    if (!group.parameters || group.parameters.length === 0) return;
    hasParams = true;
    visiblePageNum++;
    var isMixGroup = (group.name === 'Mix');

    html += '<div class="macro-group' + (isMixGroup ? ' is-mix' : '') + '" data-group="' + gi + '">';

    // Group header — Page N / Name
    html += '<div class="macro-group-header">';
    html += '<sl-icon name="chevron-down" class="macro-group-chevron"></sl-icon>';
    html += '<span class="macro-group-page-label">Page ' + visiblePageNum + '</span>';
    html += '<span class="macro-group-name">' + esc(group.name || '') + '</span>';
    html += '</div>';

    // Grid of knobs (4 columns)
    html += '<div class="macro-group-body">';
    group.parameters.forEach(function(param) {
      var value = paramValues && paramValues[param.idx] !== undefined
        ? paramValues[param.idx]
        : (param.def || 0);
      var min = param.min || 0;
      var max = param.max || 127;
      var isMacro = isMacroKnob(mappingInfo, param.idx);
      var knobColor = isMixGroup ? 'mix' : (isMacro ? 'macro' : 'normal');
      var cellClass = 'macro-knob-cell' + (isMacro ? ' is-macro' : '') + (isMixGroup ? ' is-mix' : '');

      // Name ABOVE → Knob → Value BELOW
      html += '<div class="' + cellClass + '" data-param-idx="' + param.idx + '">';
      html += '<span class="macro-knob-label">' + esc(param.name || ('P' + param.idx)) + '</span>';
      html += '<div class="macro-knob" ';
      html += 'data-value="' + value + '" data-min="' + min + '" data-max="' + max + '" data-idx="' + param.idx + '" data-color="' + knobColor + '">';
      html += renderKnobSVG({ value: value, min: min, max: max, color: knobColor, size: knobSize });
      html += '</div>';
      html += '<span class="macro-knob-value' + (isMacro ? ' is-macro' : '') + '">' + value + '</span>';

      // Target panel with range bars, value dots, display hints, badges
      var targets = mappingInfo[param.idx] || [];
      if (targets.length > 0) {
        var outputs = computeMappingOutputs(def, param.idx, value);
        html += '<div class="knob-target-panel' + (isMacro ? ' is-macro' : '') + '" data-knob-idx="' + param.idx + '">';
        if (isMacro) {
          html += '<div class="knob-target-badge">MACRO</div>';
        }
        outputs.forEach(function(o) {
          var mapping = def.mapping.find(function(mm) { return mm.ctrl === o.ctrl; });
          var rangeLow = 0, rangeHigh = 127;
          var sourceCurve = '';
          if (mapping && mapping.add) {
            if (mapping.add.length === 1) {
              rangeLow = mapping.start || 0;
              var a = mapping.add[0];
              rangeHigh = rangeLow + Math.round(127 * (a.mul || 1) / (a.div || 1));
              rangeHigh = Math.min(127, rangeHigh);
              sourceCurve = a.curve || '';
            } else {
              rangeLow = mapping.start || 0;
              rangeHigh = rangeLow;
              mapping.add.forEach(function(a) {
                rangeHigh += Math.round(127 * (a.mul || 1) / (a.div || 1));
                if (a.src === param.idx) sourceCurve = a.curve || '';
              });
              rangeHigh = Math.min(127, rangeHigh);
            }
          }
          var rangeLowPct = rangeLow / 127 * 100;
          var rangeWidthPct = (rangeHigh - rangeLow) / 127 * 100;
          var valuePct = o.value / 127 * 100;

          html += '<div class="knob-target-row" data-ctrl="' + o.ctrl + '">';
          html += '<span class="knob-target-name">' + esc(o.name) + '</span>';
          html += '<span class="knob-target-bar">';
          html += '<span class="knob-target-range" style="left:' + rangeLowPct + '%;width:' + rangeWidthPct + '%"></span>';
          html += '<span class="knob-target-dot" style="left:' + valuePct + '%"></span>';
          html += '</span>';

          // Display hint formatting
          var targetDH = window.TBD && window.TBD.displayHints;
          var targetFmt = String(o.value);
          if (targetDH && def.machine) {
            var targetParamId = def.machine + '_' + esc(o.name).replace(/[- ]/g, '_');
            var targetHint = targetDH.resolveHint(targetParamId, o.name);
            if (targetHint) {
              var physVal = targetDH.rawToDisplay(o.value, 0, 127, targetHint);
              targetFmt = targetDH.formatDisplayValue(physVal, targetHint);
            }
          }
          html += '<span class="knob-target-val">' + targetFmt + '</span>';
          // 14-bit badge
          if (mapping && mapping.bits === 14) {
            html += '<span class="knob-target-14bit">14-bit</span>';
          }
          html += '</div>';

          // Curve badge
          if (sourceCurve && sourceCurve !== 'linear') {
            html += '<span class="curve-badge">' + esc(sourceCurve) + '</span>';
          }
        });
        html += '</div>';
      }

      html += '</div>'; // .macro-knob-cell
    });
    html += '</div>'; // .macro-group-body
    html += '</div>'; // .macro-group
  });

  if (!hasParams) {
    html += '<div class="empty-state" style="padding:2rem;">';
    html += '<sl-icon name="sliders" style="font-size:2rem;"></sl-icon>';
    html += '<h3>No Parameters Defined</h3>';
    html += '<p>Add parameters in the Macro Builder to see knobs here.</p>';
    html += '</div>';
  }

  return html;
}

// ─── Active Tab State ────────────────────────────────────────
var _activeTab = 'presets';
function setActiveTab(tab) { _activeTab = tab; }
function getActiveTab() { return _activeTab; }

window.TBD = window.TBD || {};
window.TBD.shared = {
  API_V2: API_V2,
  API_TIMEOUT_MS: API_TIMEOUT_MS,
  API_MUTATION_TIMEOUT_MS: API_MUTATION_TIMEOUT_MS,
  API_PLUGIN_SWITCH_TIMEOUT_MS: API_PLUGIN_SWITCH_TIMEOUT_MS,
  esc: esc,
  formatBytes: formatBytes,
  apiFetch: apiFetch,
  apiPostJSON: apiPostJSON,
  queuedFetch: queuedFetch,
  queuedPost: queuedPost,
  FetchQueue: FetchQueue,
  paramQueue: paramQueue,
  apiQueue: apiQueue,
  toast: toast,
  iconForVariant: iconForVariant,
  setupThemeToggle: setupThemeToggle,
  applyTheme: applyTheme,
  connectionState: connectionState,
  startConnectionMonitor: startConnectionMonitor,
  setConnected: setConnected,
  setDisconnected: setDisconnected,
  updateConnectionUI: updateConnectionUI,
  confirmDialog: confirmDialog,
  isControlMode: isControlMode,
  setControlMode: setControlMode,
  loadWebAudioControls: loadWebAudioControls,
  showLoading: showLoading,
  hideLoading: hideLoading,
  // SVG knob renderer + mapping analysis
  renderKnobSVG: renderKnobSVG,
  analyzeMappings: analyzeMappings,
  isMacroKnob: isMacroKnob,
  resolveCCName: resolveCCName,
  computeMappingOutputs: computeMappingOutputs,
  renderKnobGroups: renderKnobGroups,
  // Shared data & track management
  data: sharedData,
  loadSharedData: loadSharedData,
  reloadMacroData: reloadMacroData,
  reloadFirmwareMacros: reloadFirmwareMacros,
  onTrackChange: onTrackChange,
  selectTrack: selectSharedTrack,
  getMachineInfo: getMachineInfo,
  getTrackMachines: getTrackMachines,
  renderTrackOverview: renderTrackOverview,
  setupTrackOverviewEvents: setupTrackOverviewEvents,
  setActiveTab: setActiveTab,
  getActiveTab: getActiveTab,
};

// ── display-hints.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Display Hints & Value Conversion Layer
// Vanilla JS · No dependencies
//
// Maps raw DSP parameter IDs to semantic display metadata:
//   units, physical ranges, scaling, format rules.
//
// Sources (priority order):
//   1. Macro Device JSON ui field (future, Phase 10)
//   2. mui schema extension physMin/physMax (future, firmware)
//   3. Built-in heuristic table (this file — works now)
//
// Aligned with MACRO-PRESET-SPEC.md §7 Display Types
// and PicoSeqRack PARAMTYPE enum from firmware.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {

  // ─── PARAMTYPE mapping ───────────────────────────────────
  // Maps PicoSeqRack PARAMTYPE enum values to display metadata.
  // These inform what OLED visualization is used on hardware.
  var PARAMTYPE = {
    PT_NONE:          0,
    PT_NUMBER:        1,
    PT_BIG_NUMBER:    2,
    PT_LEVEL:         10,
    PT_PAN:           11,
    PT_FILTER_TYPE:   20,
    PT_FILTER_CUTOFF: 21,
    PT_FILTER_Q:      22,
    PT_ENV_ATTACK:    30,
    PT_ENV_DECAY:     31,
    PT_ENV_AMOUNT:    32,
    PT_DISTORTION:    40,
    PT_SHAPE:         41,
    PT_SHAPE2:        42,
    PT_SHAPE3:        43,
    PT_FREQ:          44,
    PT_NOISE:         45,
    PT_HIDDEN:        100,
  };

  // ─── Heuristic Hint Table ────────────────────────────────
  // Pattern-based: matched against param ID suffix.
  // Each entry: { unit, scale, physMin, physMax, format?, widget? }
  //   scale: 'lin' | 'log'
  //   format: 'pan' | 'percent' | 'db' | 'ratio' | 'semitones' | 'select:...'
  //   widget: 'knob' | 'slider' | 'switch' | 'select' (default: 'knob')

  var SUFFIX_HINTS = [
    // Frequency parameters
    { pattern: /_f0$/,        hint: { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000, label: 'Frequency' } },
    { pattern: /_freq$/,      hint: { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000, label: 'Frequency' } },
    { pattern: /_cutoff$/,    hint: { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000, label: 'Cutoff' } },

    // Envelope time parameters
    { pattern: /_decay$/,     hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000,  label: 'Decay' } },
    { pattern: /_atk$/,       hint: { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000,  label: 'Attack' } },
    { pattern: /_attack$/,    hint: { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000,  label: 'Attack' } },
    { pattern: /_rel$/,       hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000, label: 'Release' } },
    { pattern: /_release$/,   hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000, label: 'Release' } },
    { pattern: /_sustain$/,   hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Sustain' } },

    // Level / volume
    { pattern: /_vol$/,       hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Volume', format: 'db' } },
    { pattern: /_level$/,     hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Level', format: 'db' } },
    { pattern: /_gain$/,      hint: { unit: 'dB',  scale: 'lin', physMin: -24,   physMax: 24,    label: 'Gain', format: 'db' } },

    // Panning
    { pattern: /_pan$/,       hint: { unit: '',    scale: 'lin', physMin: -100,  physMax: 100,   label: 'Pan', format: 'pan' } },

    // Filter
    { pattern: /_reso$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Resonance', format: 'percent' } },
    { pattern: /_q$/,         hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Resonance', format: 'percent' } },

    // Mix / send
    { pattern: /_mix$/,       hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Mix', format: 'percent' } },
    { pattern: /_send$/,      hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Send', format: 'db' } },
    { pattern: /_send1$/,     hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Send 1', format: 'db' } },
    { pattern: /_send2$/,     hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Send 2', format: 'db' } },

    // Tuning
    { pattern: /_tune$/,      hint: { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24,    label: 'Tune', format: 'semitones' } },
    { pattern: /_detune$/,    hint: { unit: 'ct',  scale: 'lin', physMin: -100,  physMax: 100,   label: 'Detune' } },

    // Modulation
    { pattern: /_envmod$/,    hint: { unit: '%',   scale: 'lin', physMin: -100,  physMax: 100,   label: 'Env Mod', format: 'percent' } },
    { pattern: /_envdec$/,    hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000,  label: 'Env Decay' } },
    { pattern: /_accent$/,    hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Accent', format: 'percent' } },

    // Dynamics
    { pattern: /_ratio$/,     hint: { unit: ':1',  scale: 'log', physMin: 1,     physMax: 20,    label: 'Ratio', format: 'ratio' } },
    { pattern: /_threshold$/, hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 0,     label: 'Threshold', format: 'db' } },

    // FX times
    { pattern: /_time$/,      hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 2000,  label: 'Time' } },
    { pattern: /_feedback$/,  hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Feedback', format: 'percent' } },
    { pattern: /_fb$/,        hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Feedback', format: 'percent' } },
    { pattern: /_size$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Size', format: 'percent' } },
    { pattern: /_damp$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Damping', format: 'percent' } },

    // Shape / distortion
    { pattern: /_tone$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Tone', format: 'percent' } },
    { pattern: /_shape$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Shape', format: 'percent' } },
    { pattern: /_dirty$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Dirt', format: 'percent' } },
    { pattern: /_drive$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Drive', format: 'percent' } },
    { pattern: /_noise$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Noise', format: 'percent' } },

    // Generic boolean-ish (mute, enable)
    { pattern: /_mute$/,      hint: { unit: '',    scale: 'lin', physMin: 0,     physMax: 1,     label: 'Mute', widget: 'switch' } },
    { pattern: /_enable$/,    hint: { unit: '',    scale: 'lin', physMin: 0,     physMax: 1,     label: 'Enable', widget: 'switch' } },

    // LFO
    { pattern: /_lfo_rate$/,  hint: { unit: 'Hz',  scale: 'log', physMin: 0.01,  physMax: 50,    label: 'LFO Rate' } },
    { pattern: /_lfo_amt$/,   hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'LFO Amount', format: 'percent' } },
    { pattern: /_speed$/,     hint: { unit: 'Hz',  scale: 'log', physMin: 0.01,  physMax: 50,    label: 'Speed' } },
  ];

  // ─── Name-based heuristics ───────────────────────────────
  // For params where suffix matching fails, try the display name
  var NAME_HINTS = {
    'Frequency':    { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Cutoff':       { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Freq':         { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'FREQ':         { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Decay':        { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000 },
    'DECAY':        { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000 },
    'Attack':       { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000 },
    'ATTACK':       { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000 },
    'Release':      { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000 },
    'RELEASE':      { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000 },
    'Sustain':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'SUSTAIN':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Volume':       { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Level':        { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'LEVEL':        { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Pan':          { unit: '',    scale: 'lin', physMin: -100,  physMax: 100, format: 'pan' },
    'PAN':          { unit: '',    scale: 'lin', physMin: -100,  physMax: 100, format: 'pan' },
    'Resonance':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'RESO':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Tone':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'TONE':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Mix':          { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'MIX':          { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Drive':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'DRIVE':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Feedback':     { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Noise':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'NOISE':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Accent':       { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'ACCENT':       { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Tune':         { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24, format: 'semitones' },
    'TUNE':         { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24, format: 'semitones' },
    'Detune':       { unit: 'ct',  scale: 'lin', physMin: -100,  physMax: 100 },
    'DETUNE':       { unit: 'ct',  scale: 'lin', physMin: -100,  physMax: 100 },
    'Shape':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'SHAPE':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Ratio':        { unit: ':1',  scale: 'log', physMin: 1,     physMax: 20, format: 'ratio' },
    'Threshold':    { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 0, format: 'db' },
    'Time':         { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 2000 },
    'Damping':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Size':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'FX Send 1':    { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'FX Send 2':    { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Reverb Send':  { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Reverb Level': { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Delay Level':  { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Noise Level':  { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Accent Level': { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Slide Level':  { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Overdrive':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Saturation':   { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Dirtiness':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Pitch':        { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24, format: 'semitones' },
    'Snappy':       { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Transient':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Stereo Width': { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Gain':         { unit: 'dB',  scale: 'lin', physMin: -24,   physMax: 24, format: 'db' },
    'Decimation':   { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Bit Reduction':{ unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Wave Shaping': { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Self FM':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Speed':        { unit: 'Hz',  scale: 'log', physMin: 0.01,  physMax: 50 },
    'Low Pass':     { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Slide':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Delay/ms':     { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 2000 },
  };

  // ─── Keyword-based substring matching ────────────────────
  // Matches if param name CONTAINS keyword (case-insensitive).
  // Ordered by specificity — first match wins.
  var KEYWORD_HINTS = [
    // FM synthesis
    { kw: 'frequency',  hint: { unit: 'Hz',  scale: 'log', physMin: 20,  physMax: 20000 } },
    { kw: 'cutoff',     hint: { unit: 'Hz',  scale: 'log', physMin: 20,  physMax: 20000 } },
    { kw: 'decay',      hint: { unit: 'ms',  scale: 'log', physMin: 1,   physMax: 5000 } },
    { kw: 'attack',     hint: { unit: 'ms',  scale: 'log', physMin: 0.5, physMax: 5000 } },
    { kw: 'release',    hint: { unit: 'ms',  scale: 'log', physMin: 1,   physMax: 10000 } },
    { kw: 'sustain',    hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'level',      hint: { unit: 'dB',  scale: 'lin', physMin: -60, physMax: 6, format: 'db' } },
    { kw: 'amount',     hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'index',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'feedback',   hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'envelope',   hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'vibrato',    hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'morph',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'send',       hint: { unit: 'dB',  scale: 'lin', physMin: -60, physMax: 6, format: 'db' } },
    { kw: 'filter',     hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'noise',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'drive',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'width',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'pan',        hint: { unit: '',    scale: 'lin', physMin: -100, physMax: 100, format: 'pan' } },
    { kw: 'tune',       hint: { unit: 'st',  scale: 'lin', physMin: -24,  physMax: 24, format: 'semitones' } },
    { kw: 'pitch',      hint: { unit: 'st',  scale: 'lin', physMin: -24,  physMax: 24, format: 'semitones' } },
  ];

  // ─── Resolve hint for a parameter ────────────────────────

  /**
   * Find the best display hint for a parameter.
   * @param {string} paramId - e.g. 'ch1_db_f0' or 'fx1_time'
   * @param {string} paramName - display name from schema, e.g. 'Frequency'
   * @param {object} param - full param object (may contain future physMin/physMax from mui extension)
   * @returns {object|null} hint object or null if no match
   */
  function resolveHint(paramId, paramName, param) {
    // Priority 1: Check if the param itself has physical range metadata (future mui extension)
    if (param && param.physMin !== undefined && param.physMax !== undefined) {
      return {
        unit: param.unit || '',
        scale: param.scale || 'lin',
        physMin: param.physMin,
        physMax: param.physMax,
        label: paramName,
        format: param.format || null,
      };
    }

    // Priority 2: Suffix pattern matching against param ID
    var id = paramId || '';
    for (var i = 0; i < SUFFIX_HINTS.length; i++) {
      if (SUFFIX_HINTS[i].pattern.test(id)) {
        return Object.assign({ label: paramName }, SUFFIX_HINTS[i].hint);
      }
    }

    // Priority 3: Name-based matching (exact)
    var name = (paramName || '').trim();
    if (NAME_HINTS[name]) {
      return Object.assign({ label: name }, NAME_HINTS[name]);
    }

    // Priority 4: Keyword substring matching (case-insensitive)
    var nameLower = name.toLowerCase();
    for (var k = 0; k < KEYWORD_HINTS.length; k++) {
      if (nameLower.indexOf(KEYWORD_HINTS[k].kw) !== -1) {
        return Object.assign({ label: name }, KEYWORD_HINTS[k].hint);
      }
    }

    // No match — return null (caller renders raw mode)
    return null;
  }

  // ─── Value Conversion ────────────────────────────────────

  /**
   * Convert a raw DSP value to a physical display value.
   * @param {number} rawValue - current raw value
   * @param {number} rawMin - schema min (e.g., 0)
   * @param {number} rawMax - schema max (e.g., 4095 or 127)
   * @param {object} hint - display hint from resolveHint()
   * @returns {number} physical value in display units
   */
  function rawToDisplay(rawValue, rawMin, rawMax, hint) {
    if (!hint) return rawValue;
    var range = rawMax - rawMin;
    if (range <= 0) return rawValue;
    var normalized = (rawValue - rawMin) / range;  // 0..1
    normalized = Math.max(0, Math.min(1, normalized));

    if (hint.scale === 'log' && hint.physMin > 0) {
      // Logarithmic: 0..1 → physMin..physMax log-distributed
      return hint.physMin * Math.pow(hint.physMax / hint.physMin, normalized);
    }
    // Linear: 0..1 → physMin..physMax
    return hint.physMin + normalized * (hint.physMax - hint.physMin);
  }

  /**
   * Convert a physical display value back to a raw DSP value.
   * @param {number} displayValue - value in physical units
   * @param {number} rawMin - schema min
   * @param {number} rawMax - schema max
   * @param {object} hint - display hint
   * @returns {number} raw DSP value (integer)
   */
  function displayToRaw(displayValue, rawMin, rawMax, hint) {
    if (!hint) return Math.round(displayValue);
    var range = rawMax - rawMin;
    if (range <= 0) return Math.round(displayValue);

    var normalized;
    if (hint.scale === 'log' && hint.physMin > 0) {
      normalized = Math.log(displayValue / hint.physMin) / Math.log(hint.physMax / hint.physMin);
    } else {
      var physRange = hint.physMax - hint.physMin;
      normalized = physRange !== 0 ? (displayValue - hint.physMin) / physRange : 0;
    }
    normalized = Math.max(0, Math.min(1, normalized));
    return Math.round(rawMin + normalized * range);
  }

  // ─── Value Formatting ────────────────────────────────────

  /**
   * Format a physical value for display with units.
   * @param {number} value - physical value (from rawToDisplay)
   * @param {object} hint - display hint
   * @returns {string} formatted string, e.g. "440 Hz", "L50", "+3.0 dB"
   */
  function formatDisplayValue(value, hint) {
    if (!hint) return String(Math.round(value));

    var fmt = hint.format || '';

    // Pan: L50 / C / R47
    if (fmt === 'pan') {
      if (Math.abs(value) < 1) return 'C';
      return value < 0 ? 'L' + Math.abs(Math.round(value)) : 'R' + Math.round(value);
    }

    // dB: always show sign
    if (fmt === 'db' || hint.unit === 'dB') {
      var sign = value >= 0 ? '+' : '';
      return sign + value.toFixed(1) + ' dB';
    }

    // Ratio: x:1, ∞:1 at high values
    if (fmt === 'ratio') {
      if (value > 100) return '∞:1';
      return value.toFixed(1) + ':1';
    }

    // Semitones: show sign
    if (fmt === 'semitones') {
      var st = Math.round(value);
      return (st >= 0 ? '+' : '') + st + ' st';
    }

    // Percent
    if (fmt === 'percent' || hint.unit === '%') {
      return Math.round(value) + '%';
    }

    // Hz with auto kHz
    if (hint.unit === 'Hz') {
      if (value >= 10000) return (value / 1000).toFixed(1) + ' kHz';
      if (value >= 1000) return (value / 1000).toFixed(2) + ' kHz';
      if (value >= 100) return Math.round(value) + ' Hz';
      if (value >= 10) return value.toFixed(1) + ' Hz';
      return value.toFixed(2) + ' Hz';
    }

    // ms with auto s
    if (hint.unit === 'ms') {
      if (value >= 1000) return (value / 1000).toFixed(2) + ' s';
      if (value >= 100) return Math.round(value) + ' ms';
      if (value >= 10) return value.toFixed(1) + ' ms';
      return value.toFixed(2) + ' ms';
    }

    // Cents
    if (hint.unit === 'ct') {
      var ct = Math.round(value);
      return (ct >= 0 ? '+' : '') + ct + ' ct';
    }

    // Generic with unit
    if (hint.unit) {
      return Math.round(value) + ' ' + hint.unit;
    }

    return String(Math.round(value));
  }

  // ─── Compute step for webaudio-controls ──────────────────

  /**
   * Compute a reasonable step size for a knob/slider.
   * @param {object} hint - display hint
   * @returns {number} step value
   */
  function computeStep(hint) {
    if (!hint) return 1;
    var range = Math.abs(hint.physMax - hint.physMin);
    if (range <= 1) return 0.01;
    if (range <= 10) return 0.1;
    if (range <= 100) return 1;
    if (range <= 1000) return 1;
    return Math.max(1, Math.round(range / 1000));
  }

  // ─── webaudio-controls conv expression builder ───────────
  // The `conv` attribute on <webaudio-knob> evaluates a JS expression
  // where `x` is the current value. We use it for display formatting.

  /**
   * Build the conv expression for a webaudio-controls element.
   * @param {object} hint - display hint
   * @returns {string} JS expression for conv attribute, or empty string
   */
  function buildConvExpr(hint) {
    if (!hint) return '';

    var fmt = hint.format || '';

    if (fmt === 'pan') {
      return "Math.abs(x)<1?'C':x<0?'L'+Math.abs(Math.round(x)):'R'+Math.round(x)";
    }
    if (fmt === 'db' || hint.unit === 'dB') {
      return "(x>=0?'+':'')+x.toFixed(1)+' dB'";
    }
    if (fmt === 'ratio') {
      return "x>100?'∞:1':x.toFixed(1)+':1'";
    }
    if (fmt === 'semitones') {
      return "(Math.round(x)>=0?'+':'')+Math.round(x)+' st'";
    }
    if (fmt === 'percent' || hint.unit === '%') {
      return "Math.round(x)+'%'";
    }
    if (hint.unit === 'Hz') {
      return "x>=1000?(x/1000).toFixed(1)+' kHz':x>=100?Math.round(x)+' Hz':x.toFixed(1)+' Hz'";
    }
    if (hint.unit === 'ms') {
      return "x>=1000?(x/1000).toFixed(2)+' s':x>=100?Math.round(x)+' ms':x.toFixed(1)+' ms'";
    }
    if (hint.unit === 'ct') {
      return "(Math.round(x)>=0?'+':'')+Math.round(x)+' ct'";
    }
    if (hint.unit) {
      return "Math.round(x)+' " + hint.unit + "'";
    }
    return '';
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.displayHints = {
    PARAMTYPE: PARAMTYPE,
    resolveHint: resolveHint,
    rawToDisplay: rawToDisplay,
    displayToRaw: displayToRaw,
    formatDisplayValue: formatDisplayValue,
    computeStep: computeStep,
    buildConvExpr: buildConvExpr,
  };

})();

// ── plugin-manager.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Plugin Manager
// Vanilla JS · Shoelace Web Components
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    plugins: [],          // full list from getPlugins
    activePlugin: [null, null],  // {id, name, isStereo, hint} for ch 0 and ch 1
    params: [null, null], // parameter trees for ch 0 and ch 1
    presets: [null, null], // preset lists for ch 0 and ch 1
    activePreset: [-1, -1],
    searchTerm: '',
    stereoLocked: false,      // true when slot A has a stereo plugin
    initialized: false,
    ioCaps: null,             // cached IO capabilities {t: [...], cv: [...]}
  };

  // ─── Plugin List ─────────────────────────────────────────

  /**
   * Categorize plugins by type heuristic using hint.
   * Returns Map<category, plugin[]>
   */
  function categorizePlugins(plugins) {
    var categories = new Map();
    var order = ['OSCILLATORS', 'SYNTH VOICES', 'EFFECTS', 'INSTRUMENTS', 'NOISE & GENERATORS', 'UTILITY'];

    // Keyword-based categorization from hint field
    function classify(p) {
      var hint = (p.hint || '').toLowerCase();
      var id = (p.id || '').toLowerCase();
      var name = (p.name || '').toLowerCase();

      // Utility
      if (id === 'void' || id === 'simplevca') return 'UTILITY';

      // Noise & Generators
      if (/noise|dust|bytebeat/.test(hint) || /pnoise|dust|msx|bbeats/.test(id)) return 'NOISE & GENERATORS';

      // Instruments (drums, samplers, sequencers)
      if (/drum|sample|rompl|sequen|hihat|plays back/.test(hint) || /drumrack|hihat|rompler|recnplay|picoseq/.test(id)) return 'INSTRUMENTS';

      // Effects (reverb, delay, chorus, filter, etc.)
      if (/reverb|delay|chorus|filter|diffus|ensemble|pitch\s*shift|bit\s*crush|strip|trim|effect|feedback|space|antique/.test(hint) ||
          /verb|delay|chorus|filt|strip|trim|crush|diffu|ensemble|pshft|spacefx|antique|retroact/.test(id)) return 'EFFECTS';

      // Oscillators
      if (/wavetable|oscillat|braids|sine\s*gen|svf/.test(hint) || /wtosc|macosc|sinecr|tbd03|misvf/.test(id)) return 'OSCILLATORS';

      // Default: Synth Voices
      return 'SYNTH VOICES';
    }

    // Initialize ordered categories
    order.forEach(function(cat) { categories.set(cat, []); });

    plugins.forEach(function(p) {
      var cat = classify(p);
      if (!categories.has(cat)) categories.set(cat, []);
      categories.get(cat).push(p);
    });

    // Remove empty categories
    categories.forEach(function(list, cat) {
      if (list.length === 0) categories.delete(cat);
    });

    return categories;
  }

  function renderPluginList() {
    var el = document.getElementById('plugin-list');
    var countEl = document.getElementById('plugin-count');
    var term = state.searchTerm.toLowerCase();

    var filtered = state.plugins.filter(function(p) {
      if (!term) return true;
      return p.name.toLowerCase().indexOf(term) !== -1 ||
             p.id.toLowerCase().indexOf(term) !== -1;
    });

    countEl.textContent = filtered.length + ' / ' + state.plugins.length;

    if (filtered.length === 0) {
      el.innerHTML = '<div class="param-empty">No plugins found</div>';
      return;
    }

    var categories = categorizePlugins(filtered);
    var html = '';

    categories.forEach(function(plugins, cat) {
      html += '<div class="plugin-category">' + S.esc(cat) + '</div>';
      plugins.forEach(function(p) {
        var classes = 'plugin-item';
        if (state.activePlugin[0] && state.activePlugin[0].id === p.id) {
          classes += ' active active-a';
        }
        if (state.activePlugin[1] && state.activePlugin[1].id === p.id) {
          classes += ' active active-b';
        }
        var stereo = (p.isStereo === 'true' || p.isStereo === true);
        html += '<div class="' + classes + '" data-plugin-id="' + S.esc(p.id) + '">';
        html += '<span class="plugin-item-name">' + S.esc(p.name) + '</span>';
        if (stereo) {
          html += '<span class="stereo-badge">Stereo</span>';
        } else {
          html += '<span class="mono-badge">Mono</span>';
        }
        html += '</div>';
      });
    });

    el.innerHTML = html;
  }

  function setupPluginListEvents() {
    var list = document.getElementById('plugin-list');
    var search = document.getElementById('plugin-search');

    // Search filtering
    if (search) {
      search.addEventListener('sl-input', function() {
        state.searchTerm = search.value || '';
        renderPluginList();
      });
      search.addEventListener('sl-clear', function() {
        state.searchTerm = '';
        renderPluginList();
      });
    }

    // Plugin click — load into slot
    list.addEventListener('click', function(e) {
      var item = e.target.closest('.plugin-item');
      if (!item) return;
      var pluginId = item.getAttribute('data-plugin-id');
      if (!pluginId) return;

      var pluginInfo = state.plugins.find(function(p) { return p.id === pluginId; });
      var isStereo = pluginInfo && (pluginInfo.isStereo === 'true' || pluginInfo.isStereo === true);

      // Stereo plugins always go to slot A (they occupy both)
      if (isStereo) {
        setActivePlugin(0, pluginId);
        return;
      }

      // If stereo locked, mono plugins can only go to slot A (replaces stereo)
      if (state.stereoLocked) {
        setActivePlugin(0, pluginId);
        return;
      }

      // Determine target slot for mono plugins
      if (e.shiftKey) {
        // Shift+click always targets slot B
        setActivePlugin(1, pluginId);
      } else if (!state.activePlugin[0] || !state.activePlugin[0].id || state.activePlugin[0].id === 'Void') {
        // Slot A is empty — use it
        setActivePlugin(0, pluginId);
      } else if (!state.activePlugin[1] || !state.activePlugin[1].id || state.activePlugin[1].id === 'Void') {
        // Slot A occupied, Slot B is empty — use B
        setActivePlugin(1, pluginId);
      } else {
        // Both slots occupied — show dialog
        showSlotSelectDialog(pluginId);
      }
    });
  }

  // ─── Slot Selection Dialog ────────────────────────────────

  var pendingPluginId = null;

  function showSlotSelectDialog(pluginId) {
    pendingPluginId = pluginId;
    var pluginInfo = state.plugins.find(function(p) { return p.id === pluginId; });
    var nameEl = document.getElementById('slot-select-plugin-name');
    if (nameEl) nameEl.textContent = pluginInfo ? pluginInfo.name : pluginId;
    // Disable Slot B when stereo locked
    var btnB = document.getElementById('slot-select-b');
    if (btnB) btnB.disabled = state.stereoLocked;
    document.getElementById('slot-select-dialog').show();
  }

  function setupSlotSelectDialog() {
    var btnA = document.getElementById('slot-select-a');
    var btnB = document.getElementById('slot-select-b');
    if (btnA) {
      btnA.addEventListener('click', function() {
        document.getElementById('slot-select-dialog').hide();
        if (pendingPluginId) setActivePlugin(0, pendingPluginId);
        pendingPluginId = null;
      });
    }
    if (btnB) {
      btnB.addEventListener('click', function() {
        document.getElementById('slot-select-dialog').hide();
        if (pendingPluginId) setActivePlugin(1, pendingPluginId);
        pendingPluginId = null;
      });
    }
  }

  // ─── Active Plugin Management ────────────────────────────

  var _switching = false;  // mutex — prevents re-entry during plugin switch

  // WP-F: Delays removed.  The real crash cause was Connection:close on
  // API responses (fixed in RestServer.cpp).  Without that header, the
  // browser reuses the same TCP connection (HTTP/1.1 keep-alive), so
  // sequential API calls use only 1 socket.  No artificial delays needed.

  // Plugins that use ctagSampleRom (loads ALL wavetable/sample bank data
  // from SD card into PSRAM on first use).  The firmware's setActivePlugin
  // handler BLOCKS the HTTP response until this completes — 15-30+ seconds.
  var _heavyPlugins = ['WTOsc', 'WTOscDuo', 'Freakwaves', 'VctrSnt'];

  function _isHeavyPlugin(pluginId) {
    return _heavyPlugins.indexOf(pluginId) !== -1;
  }

  async function setActivePlugin(ch, pluginId) {
    if (_switching) {
      console.warn('Plugin switch already in progress — ignoring click');
      return;
    }
    _switching = true;
    var heavy = _isHeavyPlugin(pluginId);
    S.showLoading(heavy
      ? 'Loading plugin — reading wavetable data from SD card, this may take up to 30 seconds…'
      : 'Switching plugin…');
    try {
      var pluginInfo = state.plugins.find(function(p) { return p.id === pluginId; });
      var isStereo = pluginInfo && (pluginInfo.isStereo === 'true' || pluginInfo.isStereo === true);

      // 1. Tell firmware to switch plugin (heavy: allocates plugin memory on RT core)
      // Use the longer plugin-switch timeout for heavy plugins.
      // Skip circuit breaker for this call — a timeout here means the device
      // is busy loading sample ROM from SD card, NOT that it is offline.
      var switchTimeout = heavy ? S.API_PLUGIN_SWITCH_TIMEOUT_MS : S.API_MUTATION_TIMEOUT_MS;
      try {
        await S.queuedPost(
          '/plugins?action=setActive&ch=' + ch + '&id=' + encodeURIComponent(pluginId),
          null, switchTimeout,
          true  /* skipCircuitBreaker */
        );
      } catch (firstErr) {
        // Retry once on timeout — the device may still be loading sample ROM.
        // Wait a moment then try again with a fresh timeout window.
        if (firstErr.name === 'TimeoutError' && heavy) {
          console.warn('Plugin switch timed out (' + (switchTimeout/1000) + 's) — retrying once…');
          S.showLoading('Still loading wavetable data — retrying…');
          await new Promise(function(r) { setTimeout(r, 3000); });
          await S.queuedPost(
            '/plugins?action=setActive&ch=' + ch + '&id=' + encodeURIComponent(pluginId),
            null, switchTimeout,
            true  /* skipCircuitBreaker */
          );
        } else {
          throw firstErr;
        }
      }

      // 2. Fetch the new plugin's params + presets (sequential through queue)
      await loadSlotData(ch, pluginId);

      // If loading a stereo plugin into slot A, clear slot B
      if (ch === 0 && isStereo) {
        await S.queuedPost('/plugins?action=setActive&ch=1&id=Void', null, S.API_MUTATION_TIMEOUT_MS);
        state.activePlugin[1] = null;
        state.params[1] = null;
        state.presets[1] = [];
        state.activePreset[1] = -1;
      }

      renderPluginList(); // update active indicators
    } catch (e) {
      console.error('Failed to set plugin on slot ' + (ch === 0 ? 'A' : 'B'), e);
      if (e.name === 'TimeoutError') {
        S.toast('Plugin switch timed out — the device may still be loading. Try again in a moment.', 'warning');
      } else {
        S.toast('Plugin switch failed — device may be busy or offline', 'danger');
      }
    } finally {
      S.hideLoading();
      _switching = false;
    }
  }

  async function clearSlot(ch) {
    S.showLoading('Clearing slot…');
    try {
      await S.queuedPost('/plugins?action=setActive&ch=' + ch + '&id=Void', null, S.API_MUTATION_TIMEOUT_MS);
      state.activePlugin[ch] = null;
      state.params[ch] = null;
      state.presets[ch] = [];
      state.activePreset[ch] = -1;
      renderSlotHeader(ch);
      renderEmptyCTA(ch);
      updateStereoMode();
      updateSlotVisibility();
      var presetSelect = document.getElementById('preset-select-' + ch);
      if (presetSelect) presetSelect.innerHTML = '';
      var presetList = document.getElementById('preset-list-' + ch);
      if (presetList) presetList.innerHTML = '<div class="param-empty" style="font-size:0.78rem;">No presets</div>';
      renderPluginList();
    } catch (e) {
      console.error('Failed to clear slot ' + (ch === 0 ? 'A' : 'B'), e);
      S.toast('Failed to clear slot \u2014 device may be busy or offline', 'danger');
    } finally {
      S.hideLoading();
    }
  }

  /**
   * Load slot data from the device.
   * @param {number} ch - channel (0 or 1)
   * @param {string} [knownPluginId] - if we already know the plugin ID
   *   (e.g. because we just called setActivePlugin), skip the redundant
   *   getActivePlugin round-trip.  This saves 1 API call per switch —
   *   critical on ESP32 with limited sockets and a busy RT core.
   */
  async function loadSlotData(ch, knownPluginId) {
    try {
      // Sequential fetches through global queue — ESP32 httpd has only
      // 7 sockets (4 usable). Queue ensures max 1 in-flight at a time.
      //
      // If we already know the plugin ID (caller just set it), skip the
      // redundant getActivePlugin call.  The legacy Onsen UI never made
      // this call at all — it used page navigation to separate concerns.
      var activeId;
      if (knownPluginId) {
        activeId = knownPluginId;
      } else {
        var activeData = await S.queuedFetch('/plugins?action=getActive&ch=' + ch);
        activeId = activeData.id;
      }
      var paramsData = await S.queuedFetch('/plugins?action=getParams&ch=' + ch);
      var presetsData = await S.queuedFetch('/plugins?action=getPresets&ch=' + ch);

      // Find full plugin info
      var pluginInfo = state.plugins.find(function(p) { return p.id === activeId; });
      state.activePlugin[ch] = {
        id: activeId,
        name: pluginInfo ? pluginInfo.name : activeId,
        isStereo: pluginInfo ? (pluginInfo.isStereo === 'true' || pluginInfo.isStereo === true) : false,
        hint: pluginInfo ? (pluginInfo.hint || '') : '',
      };

      state.params[ch] = paramsData;
      state.presets[ch] = presetsData.presets || [];
      state.activePreset[ch] = presetsData.activePresetNumber !== undefined
        ? presetsData.activePresetNumber : -1;

      renderSlotHeader(ch);
      renderPluginInfoBar(ch);
      renderParams(ch);
      renderPresets(ch);
      updateStereoMode();
      updateSlotVisibility();
    } catch (e) {
      console.error('Failed to load slot ' + ch + ' data:', e);
    }
  }

  // ─── Slot Header Rendering ──────────────────────────────

  function renderSlotHeader(ch) {
    var nameEl = document.getElementById('slot-name-' + ch);
    var presetPluginEl = document.getElementById('preset-plugin-' + ch);
    var removeBtn = document.getElementById('remove-plugin-' + ch);
    var subheader = document.getElementById('slot-subheader-' + ch);

    var hasPlugin = state.activePlugin[ch] && state.activePlugin[ch].id && state.activePlugin[ch].id !== 'Void';

    if (hasPlugin) {
      nameEl.textContent = state.activePlugin[ch].name;
      if (presetPluginEl) presetPluginEl.textContent = state.activePlugin[ch].name;
      if (removeBtn) removeBtn.style.display = '';
      if (subheader) subheader.style.display = '';
    } else {
      nameEl.textContent = '—';
      if (presetPluginEl) presetPluginEl.textContent = '—';
      if (removeBtn) removeBtn.style.display = 'none';
      if (subheader) subheader.style.display = 'none';
    }
  }

  // ─── Plugin Info Bar ─────────────────────────────────────

  function renderPluginInfoBar(ch) {
    var container = document.getElementById('plugin-info-' + ch);
    if (!container) return;

    var plugin = state.activePlugin[ch];
    if (!plugin || !plugin.id || plugin.id === 'Void') {
      container.innerHTML = '';
      return;
    }

    var info = state.plugins.find(function(p) { return p.id === plugin.id; });
    if (!info) {
      container.innerHTML = '';
      return;
    }

    var isStereo = (info.isStereo === 'true' || info.isStereo === true);
    var category = classifyPlugin(info);

    var html = '<div class="plugin-info-bar">';
    html += '<div class="info-badges">';
    html += '<span class="info-badge ' + (isStereo ? 'badge-stereo' : 'badge-mono') + '">' +
      (isStereo ? 'Stereo' : 'Mono') + '</span>';
    html += '<span class="info-badge badge-category">' + S.esc(category) + '</span>';
    html += '</div>';
    if (info.hint) {
      html += '<div class="info-description">' + S.esc(info.hint) + '</div>';
    }
    html += '</div>';
    container.innerHTML = html;
  }

  function classifyPlugin(p) {
    var hint = (p.hint || '').toLowerCase();
    var id = (p.id || '').toLowerCase();
    if (id === 'void' || id === 'simplevca') return 'Utility';
    if (/noise|dust|bytebeat/.test(hint) || /pnoise|dust|msx|bbeats/.test(id)) return 'Noise/Gen';
    if (/drum|sample|rompl|sequen|hihat|plays back/.test(hint) || /drumrack|hihat|rompler|recnplay|picoseq/.test(id)) return 'Instrument';
    if (/reverb|delay|chorus|filter|diffus|ensemble|pitch\s*shift|bit\s*crush|strip|trim|effect|feedback|space|antique/.test(hint) ||
        /verb|delay|chorus|filt|strip|trim|crush|diffu|ensemble|pshft|spacefx|antique|retroact/.test(id)) return 'Effect';
    if (/wavetable|oscillat|braids|sine\s*gen|svf/.test(hint) || /wtosc|macosc|sinecr|tbd03|misvf/.test(id)) return 'Oscillator';
    return 'Synth Voice';
  }

  // ─── Empty Slot CTA ──────────────────────────────────────

  function renderEmptyCTA(ch) {
    var container = document.getElementById('params-' + ch);
    if (!container) return;

    container.innerHTML = '<div class="plugin-info-container" id="plugin-info-' + ch + '"></div>' +
      '<div class="slot-empty-cta">' +
      '<div class="cta-icon"><sl-icon name="plus-lg"></sl-icon></div>' +
      '<h3>No Plugin Loaded</h3>' +
      '<p style="color:var(--sl-color-neutral-500);font-size:0.82rem;">Select a plugin from the sidebar to get started</p>' +
      '<button class="cta-btn" data-ch="' + ch + '">Add Plugin</button>' +
      '</div>';

    // Wire CTA button — open plugin sidebar if collapsed
    container.querySelector('.cta-btn').addEventListener('click', function() {
      // If stereo locked and this is Slot B, do nothing
      if (ch === 1 && state.stereoLocked) return;

      var sidebar = document.querySelector('.plugin-sidebar');
      if (sidebar && sidebar.classList.contains('collapsed')) {
        sidebar.classList.remove('collapsed');
        var icon = document.querySelector('#plugin-sidebar-toggle sl-icon');
        if (icon) icon.name = 'chevron-left';
        localStorage.setItem('tbd-plugin-sidebar-collapsed', '0');
      }
      var search = document.getElementById('plugin-search');
      if (search) search.focus();

      // Flash the slot panel to hint which slot will receive the plugin
      var panel = document.getElementById('slot-panel-' + ch);
      if (panel) {
        panel.style.outline = '2px solid var(--sl-color-primary-500)';
        panel.style.outlineOffset = '-2px';
        setTimeout(function() {
          panel.style.outline = '';
          panel.style.outlineOffset = '';
        }, 2000);
      }
    });
  }

  // ─── Stereo Mode ─────────────────────────────────────────

  function updateStereoMode() {
    var slotA = state.activePlugin[0];
    var panelA = document.getElementById('slot-panel-0');
    var panelB = document.getElementById('slot-panel-1');
    var swapBtn = document.getElementById('swap-slots-btn');

    if (slotA && slotA.isStereo) {
      state.stereoLocked = true;

      // Update slot label to A+B
      var slotLabel = panelA ? panelA.querySelector('.slot-label') : null;
      if (slotLabel) {
        slotLabel.textContent = 'A+B';
        slotLabel.classList.remove('slot-label-a');
        slotLabel.classList.add('slot-label-ab');
      }

      // Slot A gets full width
      if (panelA) panelA.classList.add('stereo-active');

      // Slot B is hidden entirely via CSS (display: none)
      if (panelB) panelB.classList.add('stereo-locked');

      // Also hide swap button
      if (swapBtn) swapBtn.classList.add('stereo-hidden');
    } else {
      state.stereoLocked = false;

      // Restore slot label to A
      var slotLabel = panelA ? panelA.querySelector('.slot-label') : null;
      if (slotLabel) {
        slotLabel.textContent = 'A';
        slotLabel.classList.remove('slot-label-ab');
        slotLabel.classList.add('slot-label-a');
      }

      // Restore normal layout
      if (panelA) panelA.classList.remove('stereo-active');

      if (panelB) {
        panelB.classList.remove('stereo-locked');

        // If Slot B is empty after unlocking, show the CTA
        var hasPluginB = state.activePlugin[1] && state.activePlugin[1].id && state.activePlugin[1].id !== 'Void';
        if (!hasPluginB) {
          renderSlotHeader(1);
          renderEmptyCTA(1);
        }
      }
      if (swapBtn) swapBtn.classList.remove('stereo-hidden');
    }
  }

  // ─── Slot Visibility (show/hide empty state) ──────────

  function updateSlotVisibility() {
    var hasA = state.activePlugin[0] && state.activePlugin[0].id && state.activePlugin[0].id !== 'Void';
    var hasB = state.activePlugin[1] && state.activePlugin[1].id && state.activePlugin[1].id !== 'Void';

    var panelA = document.getElementById('slot-panel-0');
    var panelB = document.getElementById('slot-panel-1');
    var swapBtn = document.getElementById('swap-slots-btn');

    // If neither slot has a plugin, show single unified CTA
    if (!hasA && !hasB && !state.stereoLocked) {
      // Hide slot B and swap entirely
      if (panelB) panelB.style.display = 'none';
      if (swapBtn) swapBtn.style.display = 'none';

      // Hide Slot A title bar and subheader, show full CTA
      if (panelA) {
        var titleBarA = panelA.querySelector('.slot-title-bar');
        var subheaderA = document.getElementById('slot-subheader-0');
        if (titleBarA) titleBarA.style.display = 'none';
        if (subheaderA) subheaderA.style.display = 'none';
      }
    } else {
      // Restore individual visibility
      if (!state.stereoLocked) {
        if (panelB) panelB.style.display = '';
        if (swapBtn) swapBtn.style.display = '';
      }

      for (var ch = 0; ch < 2; ch++) {
        var hasPlugin = ch === 0 ? hasA : hasB;
        var removeBtn = document.getElementById('remove-plugin-' + ch);
        var subheader = document.getElementById('slot-subheader-' + ch);
        var panel = document.getElementById('slot-panel-' + ch);
        var titleBar = panel ? panel.querySelector('.slot-title-bar') : null;

        if (titleBar) titleBar.style.display = '';
        if (hasPlugin) {
          if (removeBtn) removeBtn.style.display = '';
          if (subheader) subheader.style.display = '';
        } else {
          if (removeBtn) removeBtn.style.display = 'none';
          if (subheader) subheader.style.display = 'none';
        }
      }
    }
  }

  // ─── Parameter Rendering ─────────────────────────────────

  function renderParams(ch) {
    var container = document.getElementById('params-' + ch);
    var data = state.params[ch];

    // Preserve plugin-info-container — only replace param content
    var infoContainer = container.querySelector('.plugin-info-container');
    var infoHtml = infoContainer ? infoContainer.outerHTML : '<div class="plugin-info-container" id="plugin-info-' + ch + '"></div>';

    if (!data || !data.params || data.params.length === 0) {
      container.innerHTML = infoHtml + '<div class="param-empty">No parameters</div>';
      return;
    }

    // Wrap consecutive ungrouped params into synthetic groups
    // so the stereo two-column flex layout works for all plugins
    var normalized = [];
    var ungrouped = [];

    function flushUngrouped() {
      if (ungrouped.length > 0) {
        normalized.push({
          type: 'group',
          id: '_ungrouped_' + normalized.length,
          name: 'Parameters',
          params: ungrouped
        });
        ungrouped = [];
      }
    }

    data.params.forEach(function(p) {
      if (p.type === 'group') {
        flushUngrouped();
        normalized.push(p);
      } else {
        ungrouped.push(p);
      }
    });
    flushUngrouped();

    var html = '';
    var channelInfo = detectChannelGroups(normalized);

    if (channelInfo) {
      // Render channel containers
      channelInfo.channels.forEach(function(channel) {
        html += renderChannelContainer(ch, channel);
      });
      // Wrap remaining groups (FX/Master) in master bus container
      if (channelInfo.others.length > 0) {
        html += '<div class="master-bus-container">';
        html += '<div class="master-bus-header">';
        html += '<sl-icon name="sliders"></sl-icon>';
        html += '<span>Master Bus</span>';
        html += '</div>';
        html += '<div class="master-bus-body">';
        channelInfo.others.forEach(function(g) {
          html += renderParamGroup(ch, g);
        });
        html += '</div>';
        html += '</div>';
      }
    } else {
      // Normal rendering — no channel pattern detected
      normalized.forEach(function(p) {
        html += renderParamGroup(ch, p);
      });
    }

    container.innerHTML = infoHtml + html;
    setupParamEvents(ch, container);
    if (channelInfo) setupChannelEvents(ch, container);
  }

  // ─── Channel Group Detection & Rendering ────────────────

  /**
   * Detect if normalized groups form a channel-based layout.
   * Looks for group IDs matching ch{N}_ pattern.
   * Returns { channels: [...], others: [...] } or null.
   */
  function detectChannelGroups(normalizedGroups) {
    var channelMap = {};
    var channelOrder = [];
    var others = [];

    normalizedGroups.forEach(function(g) {
      var m = g.id.match(/^ch(\d+)_/);
      if (m) {
        var num = parseInt(m[1], 10);
        if (!channelMap[num]) {
          channelMap[num] = { num: num, mixer: null, machines: [] };
          channelOrder.push(num);
        }
        if (g.id === 'ch' + num + '_group') {
          channelMap[num].mixer = g;
        } else {
          // Extract machine index and short name from group name
          var machMatch = g.name.match(/Machine\s+(\d+)\s*-\s*(.*)/);
          var idx, shortName;
          if (machMatch) {
            idx = parseInt(machMatch[1], 10);
            shortName = machMatch[2].trim();
          } else {
            idx = channelMap[num].machines.length;
            // Remove "Channel N - " prefix if present
            shortName = g.name.replace(/^Channel\s+\d+\s*-\s*/, '');
          }
          channelMap[num].machines.push({ group: g, idx: idx, name: shortName });
        }
      } else {
        others.push(g);
      }
    });

    if (channelOrder.length === 0) return null;

    var channels = channelOrder.map(function(num) { return channelMap[num]; });
    return { channels: channels, others: others };
  }

  /**
   * Render a channel container: engine tabs → machine params → mixer strip.
   * Layout follows audio console convention: primary controls (engine) on top,
   * mixer strip (level/pan/sends) at bottom.
   */
  function renderChannelContainer(ch, channel) {
    var mixer = channel.mixer;
    var machines = channel.machines;

    // Extract channel display name from mixer group name
    // "Channel 1 - Drum group - Kicks" → name:"Kicks", category:"Drum group"
    var parts = mixer ? mixer.name.split(' - ') : [];
    var channelName = parts.length >= 3 ? parts.slice(2).join(' - ') :
                      parts.length >= 2 ? parts[1] : 'Channel ' + channel.num;
    var categoryName = parts.length >= 3 ? parts[1] : '';
    var displayNum = String(channel.num).padStart(2, '0');

    var html = '<div class="channel-container" data-channel="' + channel.num + '">';

    // Find Mute param in mixer for header rendering
    var muteParam = null;
    if (mixer && mixer.params) {
      for (var mi = 0; mi < mixer.params.length; mi++) {
        if (mixer.params[mi].name === 'Mute') {
          muteParam = mixer.params[mi];
          break;
        }
      }
    }

    // Channel header
    html += '<div class="channel-header">';
    html += '<sl-icon name="chevron-down" class="channel-chevron"></sl-icon>';
    html += '<span class="channel-num">CH ' + displayNum + '</span>';
    html += '<span class="channel-name">' + S.esc(channelName) + '</span>';
    if (categoryName) {
      html += '<span class="channel-category">' + S.esc(categoryName) + '</span>';
    }
    if (muteParam) {
      var muteChecked = muteParam.current ? ' checked' : '';
      html += '<sl-switch size="small" class="channel-mute-switch"' + muteChecked + ' ';
      html += 'data-param-id="' + S.esc(muteParam.id) + '" data-ch="' + ch + '" ';
      html += 'title="Mute">';
      html += '</sl-switch>';
    }
    html += '</div>';

    // Channel body
    html += '<div class="channel-body">';

    // Show engine tabs if channel has multiple machines
    var hasSelector = machines.length > 1;

    // Find Device param and determine active machine
    var deviceParam = null;
    if (mixer && mixer.params) {
      for (var i = 0; i < mixer.params.length; i++) {
        if (mixer.params[i].name === 'Device') {
          deviceParam = mixer.params[i];
          break;
        }
      }
    }
    var deviceValue = deviceParam ? (deviceParam.current || 0) : 0;
    var activeMachineIdx = 0;
    if (hasSelector && deviceParam) {
      var maxVal = deviceParam.max || 4095;
      activeMachineIdx = deviceValue > (maxVal / 2) ? 1 : 0;
    }

    // ── Engine tab bar (only if multiple machines)
    if (hasSelector) {
      html += '<div class="machine-tab-bar" data-channel="' + channel.num + '">';
      machines.forEach(function(m, tabIdx) {
        var isActive = tabIdx === activeMachineIdx;
        html += '<button class="machine-tab' + (isActive ? ' active' : '') + '" ';
        html += 'data-machine-idx="' + tabIdx + '" data-channel="' + channel.num + '">';
        html += S.esc(m.name);
        html += '</button>';
      });
      html += '</div>';
    }

    // ── Machine param panels (render params directly, no nested group chrome)
    machines.forEach(function(m, tabIdx) {
      var isVisible = hasSelector ? (tabIdx === activeMachineIdx) : true;
      var machineGridCls = S.isControlMode() ? ' knob-grid' : '';
      html += '<div class="machine-params' + machineGridCls + '" data-machine-idx="' + tabIdx + '" data-channel="' + channel.num + '"';
      if (!isVisible) html += ' style="display:none;"';
      html += '>';
      // Render params directly — no collapsible group wrapper
      if (m.group.params) {
        var machParamIdx = 0;
        m.group.params.forEach(function(p) {
          html += renderParamRow(ch, p);
          machParamIdx++;
          if (S.isControlMode() && machParamIdx % 4 === 0) {
            html += '<div class="knob-row-divider"></div>';
          }
        });
      }
      html += '</div>';
    });

    // ── Mixer strip (bottom — level, pan, sends)
    if (mixer && mixer.params) {
      html += '<div class="channel-mixer">';
      html += '<div class="channel-mixer-label">';
      html += '<sl-icon name="sliders"></sl-icon>';
      html += '<span>Track Mix</span>';
      html += '<sl-icon name="chevron-down" class="mixer-chevron"></sl-icon>';
      html += '</div>';
      var mixerGridCls = S.isControlMode() ? ' knob-grid' : '';
      html += '<div class="channel-mixer-body' + mixerGridCls + '">';
      var mixerParamIdx = 0;
      mixer.params.forEach(function(p) {
        // Skip Device param — it's an engine selector, not a mixer control
        if (p.name === 'Device') return;
        // Skip Mute — moved to channel header
        if (p.name === 'Mute') return;
        html += renderParamRow(ch, p);
        mixerParamIdx++;
        if (S.isControlMode() && mixerParamIdx % 4 === 0) {
          html += '<div class="knob-row-divider"></div>';
        }
      });
      html += '</div>';
      html += '</div>';
    }

    html += '</div>'; // .channel-body
    html += '</div>'; // .channel-container
    return html;
  }

  /**
   * Set up event handlers for channel containers:
   * - Channel header collapse/expand
   * - Machine tab switching
   */
  function setupChannelEvents(ch, container) {
    // Channel collapse/expand
    container.querySelectorAll('.channel-header').forEach(function(header) {
      header.addEventListener('click', function(e) {
        // Don't toggle collapse when clicking the mute switch
        if (e.target.closest('.channel-mute-switch')) return;
        header.parentElement.classList.toggle('collapsed');
      });
    });

    // Channel mute switches in header
    container.querySelectorAll('.channel-mute-switch').forEach(function(sw) {
      sw.addEventListener('sl-change', function(e) {
        e.stopPropagation();
        var paramId = sw.getAttribute('data-param-id');
        var slotCh = parseInt(sw.getAttribute('data-ch'), 10);
        sendParamValue(slotCh, paramId, sw.checked ? 1 : 0);
      });
    });

    // Mixer collapse/expand
    container.querySelectorAll('.channel-mixer-label').forEach(function(label) {
      label.addEventListener('click', function() {
        label.parentElement.classList.toggle('mixer-collapsed');
      });
    });

    // Machine tab switching
    container.querySelectorAll('.machine-tab').forEach(function(tab) {
      tab.addEventListener('click', function(e) {
        e.stopPropagation();
        var channelNum = tab.getAttribute('data-channel');
        var tabIdx = parseInt(tab.getAttribute('data-machine-idx'), 10);
        var channelEl = tab.closest('.channel-container');

        // Update active tab
        channelEl.querySelectorAll('.machine-tab').forEach(function(t) {
          t.classList.toggle('active', t === tab);
        });

        // Show/hide machine params
        channelEl.querySelectorAll('.machine-params').forEach(function(mp) {
          var idx = parseInt(mp.getAttribute('data-machine-idx'), 10);
          mp.style.display = (idx === tabIdx) ? '' : 'none';
        });

        // Send Device param value
        var deviceParamId = 'ch' + channelNum + '_device';
        // Map tab index to Device value: 0 → 0, 1 → max (4095)
        var deviceValue = tabIdx === 0 ? 0 : 4095;
        sendParamValue(ch, deviceParamId, deviceValue);
      });
    });
  }

  // ─── Param Group & Row Rendering ────────────────────────

  function renderParamGroup(ch, group, nested) {
    var controlMode = S.isControlMode();
    var cls = nested ? 'param-group param-group-nested' : 'param-group';
    var html = '<div class="' + cls + '" data-group-id="' + S.esc(group.id) + '">';
    html += '<div class="param-group-header">';
    html += '<sl-icon name="chevron-down" class="param-group-chevron"></sl-icon>';
    html += '<span class="param-group-name" title="' + S.esc(group.name) + '">' + S.esc(group.name) + '</span>';
    html += '</div>';
    html += '<div class="param-group-body' + (controlMode ? ' knob-grid' : '') + '">';

    if (group.params) {
      var groupParamIdx = 0;
      group.params.forEach(function(p) {
        if (p.type === 'group') {
          // Recursively render nested groups
          html += renderParamGroup(ch, p, true);
        } else {
          html += renderParamRow(ch, p);
          groupParamIdx++;
          if (controlMode && groupParamIdx % 4 === 0) {
            html += '<div class="knob-row-divider"></div>';
          }
        }
      });
    }

    html += '</div>';
    html += '</div>';
    return html;
  }

  function renderParamRow(ch, param) {
    // If a group somehow ends up here, render it as a nested group
    if (param.type === 'group') {
      return renderParamGroup(ch, param, true);
    }

    var controlMode = S.isControlMode();
    var knobClass = controlMode ? ' knob-mode' : '';

    var html = '<div class="param-row' + knobClass + '" data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '">';
    html += '<span class="param-name" title="' + S.esc(param.name) + '">' + S.esc(param.name) + '</span>';
    html += '<div class="param-control">';

    if (param.type === 'bool') {
      html += renderBoolParam(ch, param);
    } else if (controlMode) {
      html += renderKnobParam(ch, param);
    } else {
      // int or float — Config Mode (raw sliders)
      html += renderSliderParam(ch, param);
    }

    html += '</div>';

    // CV/TRIG routing dropdown (Config Mode only — not shown in Control Mode knob grid)
    if (!controlMode && state.ioCaps) {
      html += renderCVTrigDropdown(ch, param);
    }

    html += '</div>';

    return html;
  }

  function renderSliderParam(ch, param) {
    var min = param.min !== undefined ? param.min : 0;
    var max = param.max !== undefined ? param.max : 4095;
    var step = param.step !== undefined ? param.step : 1;
    var current = param.current !== undefined ? param.current : min;

    var html = '<input type="range" class="param-slider" ';
    html += 'min="' + min + '" max="' + max + '" step="' + step + '" ';
    html += 'value="' + current + '" ';
    html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '">';
    html += '<span class="param-value">' + current + '</span>';
    return html;
  }

  function renderBoolParam(ch, param) {
    var checked = param.current ? ' checked' : '';
    var html = '<div class="param-bool">';
    html += '<sl-switch size="small"' + checked + ' ';
    html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '">';
    html += '</sl-switch>';
    html += '</div>';
    return html;
  }

  function renderKnobParam(ch, param) {
    var DH = window.TBD.displayHints;
    var min = param.min !== undefined ? param.min : 0;
    var max = param.max !== undefined ? param.max : 4095;
    var current = param.current !== undefined ? param.current : min;

    var hint = DH ? DH.resolveHint(param.id, param.name, param) : null;

    if (hint) {
      // Control Mode: knob operates in physical units
      var physMin = hint.physMin;
      var physMax = hint.physMax;
      var step = DH.computeStep(hint);
      var displayVal = DH.rawToDisplay(current, min, max, hint);
      var formatted = DH.formatDisplayValue(displayVal, hint);

      var html = '<webaudio-knob class="param-knob" ';
      html += 'diameter="52" ';
      html += 'min="' + physMin + '" max="' + physMax + '" ';
      html += 'step="' + step + '" ';
      html += 'value="' + displayVal.toFixed(4) + '" ';
      html += 'colors="#ccc;#484848;#525252" ';
      html += 'valuetip="0" sensitivity="0.5" ';
      html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '" ';
      html += 'data-raw-min="' + min + '" data-raw-max="' + max + '" ';
      html += 'data-hint="1">';
      html += '</webaudio-knob>';
      html += '<span class="param-value">' + S.esc(formatted) + '</span>';
      return html;
    }

    // No hint: render a knob with raw values
    var html = '<webaudio-knob class="param-knob" ';
    html += 'diameter="52" ';
    html += 'min="' + min + '" max="' + max + '" ';
    html += 'step="1" ';
    html += 'value="' + current + '" ';
    html += 'colors="#ccc;#484848;#525252" ';
    html += 'valuetip="0" sensitivity="0.5" ';
    html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '" ';
    html += 'data-raw-min="' + min + '" data-raw-max="' + max + '">';
    html += '</webaudio-knob>';
    html += '<span class="param-value">' + current + '</span>';
    return html;
  }

  // ─── CV / TRIG Routing Dropdown ─────────────────────────

  /**
   * Render a compact CV or TRIG assignment dropdown for a parameter.
   * - int params get a CV dropdown (sources from ioCaps.cv)
   * - bool params get a TRIG dropdown (sources from ioCaps.t)
   */
  function renderCVTrigDropdown(ch, param) {
    if (!state.ioCaps) return '';

    var isBool = param.type === 'bool';
    var sources = isBool ? (state.ioCaps.t || []) : (state.ioCaps.cv || []);
    var currentIdx = isBool ? (param.trig !== undefined ? param.trig : -1)
                            : (param.cv !== undefined ? param.cv : -1);
    var label = isBool ? 'TRIG' : 'CV';
    var dataAttr = isBool ? 'data-trig-param' : 'data-cv-param';

    var html = '<div class="param-routing">';
    var assignedClass = currentIdx >= 0 ? ' assigned' : '';
    html += '<select class="param-routing-select' + assignedClass + '" ' + dataAttr + '="' + S.esc(param.id) + '" data-ch="' + ch + '" title="' + label + ' routing for ' + S.esc(param.name) + '">';
    html += '<option value="-1"' + (currentIdx === -1 ? ' selected' : '') + '>—</option>';
    for (var i = 0; i < sources.length; i++) {
      var selected = (i === currentIdx) ? ' selected' : '';
      html += '<option value="' + i + '"' + selected + '>' + S.esc(sources[i]) + '</option>';
    }
    html += '</select>';
    html += '</div>';
    return html;
  }

  // ─── Parameter Events ───────────────────────────────────

  function setupParamEvents(ch, container) {
    // Group collapse/expand
    container.querySelectorAll('.param-group-header').forEach(function(header) {
      header.addEventListener('click', function() {
        header.parentElement.classList.toggle('collapsed');
      });
    });

    // Slider input (live update value display)
    container.querySelectorAll('.param-slider').forEach(function(slider) {
      var valueEl = slider.nextElementSibling;

      slider.addEventListener('input', function() {
        if (valueEl) valueEl.textContent = slider.value;
      });

      // Send on change (release or final value)
      slider.addEventListener('change', function() {
        var paramId = slider.getAttribute('data-param-id');
        var slotCh = parseInt(slider.getAttribute('data-ch'), 10);
        sendParamValue(slotCh, paramId, slider.value);
      });
    });

    // Boolean switches
    container.querySelectorAll('sl-switch[data-param-id]').forEach(function(sw) {
      sw.addEventListener('sl-change', function() {
        var paramId = sw.getAttribute('data-param-id');
        var slotCh = parseInt(sw.getAttribute('data-ch'), 10);
        sendParamValue(slotCh, paramId, sw.checked ? 1 : 0);
      });
    });

    // webaudio-knob elements (Control Mode)
    container.querySelectorAll('webaudio-knob[data-param-id]').forEach(function(knob) {
      var valueEl = knob.nextElementSibling;
      var DH = window.TBD.displayHints;
      var hasHint = knob.getAttribute('data-hint') === '1';
      var rawMin = parseInt(knob.getAttribute('data-raw-min'), 10) || 0;
      var rawMax = parseInt(knob.getAttribute('data-raw-max'), 10) || 4095;
      var paramId = knob.getAttribute('data-param-id');
      var paramName = '';
      var nameEl = knob.closest('.param-row');
      if (nameEl) {
        var nameSpan = nameEl.querySelector('.param-name');
        if (nameSpan) paramName = nameSpan.textContent;
      }
      var hint = hasHint && DH ? DH.resolveHint(paramId, paramName) : null;

      // Suppress hover on other param-rows while dragging this knob
      var paramRow = knob.closest('.param-row');
      var isDragging = false;

      // Live update display value on input
      knob.addEventListener('input', function() {
        // Activate drag suppression on first input
        if (!isDragging) {
          isDragging = true;
          document.body.classList.add('knob-dragging');
          if (paramRow) paramRow.classList.add('knob-active');
        }
        if (!valueEl) return;
        if (hint && DH) {
          valueEl.textContent = DH.formatDisplayValue(parseFloat(knob.value), hint);
        } else {
          valueEl.textContent = Math.round(parseFloat(knob.value));
        }
      });

      // Send raw value on change (release)
      knob.addEventListener('change', function() {
        isDragging = false;
        document.body.classList.remove('knob-dragging');
        if (paramRow) paramRow.classList.remove('knob-active');
        var slotCh = parseInt(knob.getAttribute('data-ch'), 10);
        var rawValue;
        if (hint && DH) {
          rawValue = DH.displayToRaw(parseFloat(knob.value), rawMin, rawMax, hint);
        } else {
          rawValue = Math.round(parseFloat(knob.value));
        }
        sendParamValue(slotCh, paramId, rawValue);
      });
    });

    // CV routing selects
    container.querySelectorAll('select[data-cv-param]').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var paramId = sel.getAttribute('data-cv-param');
        var slotCh = parseInt(sel.getAttribute('data-ch'), 10);
        var val = parseInt(sel.value, 10);
        sel.classList.toggle('assigned', val >= 0);
        sendCVValue(slotCh, paramId, val);
      });
    });

    // TRIG routing selects
    container.querySelectorAll('select[data-trig-param]').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var paramId = sel.getAttribute('data-trig-param');
        var slotCh = parseInt(sel.getAttribute('data-ch'), 10);
        var val = parseInt(sel.value, 10);
        sel.classList.toggle('assigned', val >= 0);
        sendTrigValue(slotCh, paramId, val);
      });
    });

  }

  // ─── Debounced Param Sending ─────────────────────────────
  // Per-param debounce timers: rapid knob/slider changes collapse into
  // a single API call (WLED pattern — 50ms debounce on continuous input).
  var _paramTimers = {};
  var PARAM_DEBOUNCE_MS = 50;

  function sendParamValue(ch, paramId, value) {
    // Notify debug panel
    if (S.onParamChange) S.onParamChange(ch, paramId, value);

    // Debounce: cancel previous pending send for this exact param
    var key = ch + ':' + paramId;
    if (_paramTimers[key]) clearTimeout(_paramTimers[key]);

    _paramTimers[key] = setTimeout(function() {
      delete _paramTimers[key];
      // Route directly through apiQueue — no need for double-serialization
      // through paramQueue→apiQueue.  Single queue is sufficient and
      // reduces overhead on the constrained ESP32 httpd.
      S.queuedPost('/plugins?action=setParam&ch=' + ch + '&id=' +
        encodeURIComponent(paramId) + '&key=current&val=' + encodeURIComponent(value),
        null
      ).catch(function(e) {
        console.error('Failed to set param:', paramId, e);
      });
    }, PARAM_DEBOUNCE_MS);
  }

  function sendCVValue(ch, paramId, value) {
    S.queuedPost('/plugins?action=setParam&ch=' + ch + '&id=' +
      encodeURIComponent(paramId) + '&key=cv&val=' + encodeURIComponent(value),
      null
    ).catch(function(e) {
      console.error('Failed to set CV:', paramId, e);
    });
  }

  function sendTrigValue(ch, paramId, value) {
    S.queuedPost('/plugins?action=setParam&ch=' + ch + '&id=' +
      encodeURIComponent(paramId) + '&key=trig&val=' + encodeURIComponent(value),
      null
    ).catch(function(e) {
      console.error('Failed to set TRIG:', paramId, e);
    });
  }


  // ─── Preset Rendering ───────────────────────────────────

  function renderPresets(ch) {
    // Inline preset select in slot title bar
    var selectEl = document.getElementById('preset-select-' + ch);
    if (selectEl) {
      var html = '';
      if (state.presets[ch] && state.presets[ch].length > 0) {
        state.presets[ch].forEach(function(p) {
          html += '<sl-option value="' + p.number + '">' +
            S.esc(p.name || ('Preset ' + p.number)) + '</sl-option>';
        });
      }
      selectEl.innerHTML = html;
      if (state.activePreset[ch] >= 0) {
        selectEl.value = String(state.activePreset[ch]);
      }
    }

    // Sidebar preset list — removed (using in-slot preset controls only)
  }

  function setupPresetEvents() {
    // Preset selection from slot title selects
    for (var ch = 0; ch < 2; ch++) {
      (function(ch) {
        var sel = document.getElementById('preset-select-' + ch);
        if (sel) {
          sel.addEventListener('sl-change', function() {
            var num = parseInt(sel.value, 10);
            if (!isNaN(num)) loadPreset(ch, num);
          });
        }

        var saveBtn = document.getElementById('save-preset-' + ch);
        if (saveBtn) {
          saveBtn.addEventListener('click', function() {
            openSavePresetDialog(ch);
          });
        }
      })(ch);
    }
  }

  async function loadPreset(ch, number) {
    S.showLoading('Loading preset…');
    try {
      await S.queuedPost('/plugins?action=loadPreset&ch=' + ch + '&number=' + number, null, S.API_MUTATION_TIMEOUT_MS);
      state.activePreset[ch] = number;
      // Reload params to reflect preset values
      var paramsData = await S.queuedFetch('/plugins?action=getParams&ch=' + ch);
      state.params[ch] = paramsData;
      renderParams(ch);
      renderPresets(ch);
    } catch (e) {
      console.error('Failed to load preset on slot ' + (ch === 0 ? 'A' : 'B'), e);
    } finally {
      S.hideLoading();
    }
  }

  // ─── Save Preset Dialog ──────────────────────────────────

  var savePresetCh = 0;

  function openSavePresetDialog(ch) {
    savePresetCh = ch;
    var nameInput = document.getElementById('save-preset-name');
    var slotSelect = document.getElementById('save-preset-slot');

    nameInput.value = '';

    // Populate slot options — dynamic count based on existing presets
    // Show all existing slots + 1 extra slot for creating a new preset
    var maxSlot = 0;
    if (state.presets[ch] && state.presets[ch].length > 0) {
      state.presets[ch].forEach(function(p) {
        if (p.number >= maxSlot) maxSlot = p.number + 1;
      });
    }
    var slotCount = Math.max(maxSlot + 1, 10); // minimum 10 slots, always offer +1
    var html = '';
    for (var i = 0; i < slotCount; i++) {
      var preset = state.presets[ch] ? state.presets[ch].find(function(p) {
        return p.number === i;
      }) : null;
      var label = i + (preset ? ' — ' + S.esc(preset.name) : ' — (empty)');
      var selected = (i === state.activePreset[ch]) ? ' selected' : '';
      html += '<sl-option value="' + i + '">' + label + '</sl-option>';
    }
    slotSelect.innerHTML = html;
    if (state.activePreset[ch] >= 0) {
      slotSelect.value = String(state.activePreset[ch]);
    } else {
      slotSelect.value = '0';
    }

    document.getElementById('save-preset-dialog').show();
  }

  function setupSavePresetDialog() {
    var okBtn = document.getElementById('save-preset-ok');
    var cancelBtn = document.getElementById('save-preset-cancel');

    if (okBtn) {
      okBtn.addEventListener('click', async function() {
        var name = document.getElementById('save-preset-name').value.trim();
        var slot = parseInt(document.getElementById('save-preset-slot').value, 10);
        if (!name) {
          S.toast('Enter a preset name', 'warning');
          return;
        }
        try {
          await S.queuedPost('/plugins?action=savePreset&ch=' + savePresetCh +
            '&number=' + slot + '&name=' + encodeURIComponent(name), null);
          document.getElementById('save-preset-dialog').hide();
          // Reload presets
          var presetsData = await S.queuedFetch('/plugins?action=getPresets&ch=' + savePresetCh);
          state.presets[savePresetCh] = presetsData.presets || [];
          state.activePreset[savePresetCh] = slot;
          renderPresets(savePresetCh);
          S.toast('Preset saved: ' + name, 'success');
        } catch (e) {
          S.toast('Failed to save preset', 'danger');
        }
      });
    }
    if (cancelBtn) {
      cancelBtn.addEventListener('click', function() {
        document.getElementById('save-preset-dialog').hide();
      });
    }
  }

  // ─── Favorites Bar (Header) ──────────────────────────────

  var favoritesCache = null;

  function setupFavoritesBar() {
    var bar = document.getElementById('favorites-bar');
    if (!bar) return;

    // Load favorites data to populate tooltips
    loadFavoritesCache();

    // Close any open popover when clicking outside
    document.addEventListener('click', function(e) {
      if (!e.target.closest('.fav-slot-btn') && !e.target.closest('.fav-popover')) {
        closeFavPopover();
      }
    });

    bar.addEventListener('click', function(e) {
      // Handle popover button clicks
      var popBtn = e.target.closest('.fav-popover-btn');
      if (popBtn) {
        e.stopPropagation();
        var action = popBtn.getAttribute('data-action');
        var idx = parseInt(popBtn.getAttribute('data-fav'), 10);
        closeFavPopover();
        if (action === 'recall') {
          recallFavorite(idx);
        } else if (action === 'store') {
          storeFavorite(idx);
        } else if (action === 'export-all') {
          exportFavorites();
        } else if (action === 'import-all') {
          importFavorites();
        }
        return;
      }

      var btn = e.target.closest('.fav-slot-btn');
      if (!btn) return;
      var idx = parseInt(btn.getAttribute('data-fav'), 10);

      // Close existing popover if clicking same button
      var existing = btn.querySelector('.fav-popover');
      if (existing) {
        closeFavPopover();
        return;
      }

      // Close any other open popover first
      closeFavPopover();

      // Show popover
      showFavPopover(btn, idx);
    });
  }

  function closeFavPopover() {
    var pop = document.querySelector('.fav-popover');
    if (pop) pop.remove();
  }

  function showFavPopover(btn, idx) {
    var fav = favoritesCache ? favoritesCache[idx] : null;
    var hasData = fav && fav.plug_0 && fav.plug_0 !== 'Void';

    var infoText = hasData
      ? 'A: ' + fav.plug_0 + (fav.plug_1 && fav.plug_1 !== 'Void' ? ' | B: ' + fav.plug_1 : '')
      : 'Empty slot';

    var pop = document.createElement('div');
    pop.className = 'fav-popover';
    pop.innerHTML =
      '<button class="fav-popover-btn' + (hasData ? '' : ' disabled') + '" data-action="recall" data-fav="' + idx + '"' + (hasData ? '' : ' disabled') + '>' +
        '<sl-icon name="play-fill"></sl-icon> Recall' +
      '</button>' +
      '<button class="fav-popover-btn store" data-action="store" data-fav="' + idx + '">' +
        '<sl-icon name="floppy"></sl-icon> Store Current' +
      '</button>' +
      '<div class="fav-popover-divider"></div>' +
      '<button class="fav-popover-btn" data-action="export-all" data-fav="' + idx + '">' +
        '<sl-icon name="download"></sl-icon> Export All' +
      '</button>' +
      '<button class="fav-popover-btn" data-action="import-all" data-fav="' + idx + '">' +
        '<sl-icon name="upload"></sl-icon> Import All' +
      '</button>' +
      '<div class="fav-popover-info">' + S.esc(infoText) + '</div>';

    btn.appendChild(pop);
  }

  async function loadFavoritesCache() {
    try {
      favoritesCache = await S.queuedFetch('/device?action=getFavorites');
      updateFavoritesBarTooltips();
    } catch (e) {
      favoritesCache = null;
    }
  }

  function updateFavoritesBarTooltips() {
    if (!favoritesCache) return;
    var btns = document.querySelectorAll('.fav-slot-btn');
    btns.forEach(function(btn) {
      var idx = parseInt(btn.getAttribute('data-fav'), 10);
      var fav = favoritesCache[idx];
      if (fav && fav.plug_0 && fav.plug_0 !== 'Void') {
        btn.title = 'Fav ' + (idx + 1) + ': A=' + fav.plug_0 + ' B=' + (fav.plug_1 || 'Void');
        btn.classList.add('has-data');
      } else {
        btn.title = 'Fav ' + (idx + 1) + ': (empty)';
        btn.classList.remove('has-data');
      }
    });
  }

  async function recallFavorite(idx) {
    S.showLoading('Recalling favorite…');
    try {
      await S.queuedPost('/device?action=recallFavorite&id=' + idx, null, S.API_MUTATION_TIMEOUT_MS);
      // Sequential — never use Promise.all against ESP32
      await loadSlotData(0);
      await loadSlotData(1);
      renderPluginList();
      await loadFavoritesCache();
    } catch (e) {
      console.error('Failed to recall favorite ' + (idx + 1), e);
    } finally {
      S.hideLoading();
    }
  }

  async function storeFavorite(idx) {
    var favData = {
      name: 'Favorite ' + (idx + 1),
      plug_0: state.activePlugin[0] ? state.activePlugin[0].id : 'Void',
      pre_0:  state.activePreset[0] >= 0 ? state.activePreset[0] : 0,
      plug_1: state.activePlugin[1] ? state.activePlugin[1].id : 'Void',
      pre_1:  state.activePreset[1] >= 0 ? state.activePreset[1] : 0,
      ustring: '',
    };
    try {
      await S.queuedPost('/device?action=storeFavorite&id=' + idx, favData);
      S.toast('Stored favorite ' + (idx + 1), 'success', 2000);
      await loadFavoritesCache();
    } catch (e) {
      S.toast('Failed to store favorite ' + (idx + 1), 'danger');
    }
  }

  async function exportFavorites() {
    try {
      if (!favoritesCache) await loadFavoritesCache();
      var data = JSON.stringify(favoritesCache, null, 2);
      var blob = new Blob([data], { type: 'application/json' });
      var url = URL.createObjectURL(blob);
      var a = document.createElement('a');
      a.href = url;
      a.download = 'ctag-tbd-favorites.json';
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      S.toast('Favorites exported', 'success', 2000);
    } catch (e) {
      S.toast('Export failed: ' + e.message, 'danger');
    }
  }

  async function importFavorites() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', async function() {
      if (!input.files || !input.files[0]) return;
      try {
        var text = await input.files[0].text();
        var data = JSON.parse(text);
        if (!Array.isArray(data)) {
          S.toast('Invalid favorites file — expected array', 'danger');
          return;
        }
        S.showLoading('Importing favorites…');
        for (var i = 0; i < data.length; i++) {
          if (data[i] && data[i].plug_0) {
            await S.queuedPost('/device?action=storeFavorite&id=' + i, data[i]);
          }
        }
        await loadFavoritesCache();
        S.hideLoading();
        S.toast('Imported ' + data.length + ' favorites', 'success', 2000);
      } catch (e) {
        S.hideLoading();
        S.toast('Import failed: ' + e.message, 'danger');
      }
    });
    input.click();
  }

  // ─── Swap Slots ──────────────────────────────────────────

  function setupSwapSlots() {
    var btn = document.getElementById('swap-slots-btn');
    if (!btn) return;

    btn.addEventListener('click', async function() {
      if (state.stereoLocked) {
        S.toast('Cannot swap — stereo plugin active', 'warning');
        return;
      }

      var plugA = state.activePlugin[0] ? state.activePlugin[0].id : 'Void';
      var plugB = state.activePlugin[1] ? state.activePlugin[1].id : 'Void';

      if (plugA === 'Void' && plugB === 'Void') {
        S.toast('Both slots are empty', 'warning', 2000);
        return;
      }

      try {
        S.showLoading('Swapping slots…');
        // Set A to B's plugin and B to A's plugin
        await S.queuedPost('/plugins?action=setActive&ch=0&id=' + encodeURIComponent(plugB), null, S.API_MUTATION_TIMEOUT_MS);
        await S.queuedPost('/plugins?action=setActive&ch=1&id=' + encodeURIComponent(plugA), null, S.API_MUTATION_TIMEOUT_MS);
        // Sequential — never use Promise.all against ESP32
        await loadSlotData(0, plugB);
        await loadSlotData(1, plugA);
        renderPluginList();
      } catch (e) {
        console.error('Failed to swap slots', e);
        S.toast('Swap failed \u2014 device may be busy or offline', 'danger');
      } finally {
        S.hideLoading();
      }
    });
  }

  // ─── Sidebar Collapse/Expand ──────────────────────────────

  function setupSidebarToggles() {
    // Plugin sidebar (left)
    var pluginToggle = document.getElementById('plugin-sidebar-toggle');
    var pluginSidebar = document.querySelector('.plugin-sidebar');
    if (pluginToggle && pluginSidebar) {
      var pluginIcon = pluginToggle.querySelector('sl-icon');
      // Restore state from localStorage
      if (localStorage.getItem('tbd-plugin-sidebar-collapsed') === '1') {
        pluginSidebar.classList.add('collapsed');
        if (pluginIcon) pluginIcon.name = 'chevron-right';
      }
      pluginToggle.addEventListener('click', function() {
        pluginSidebar.classList.toggle('collapsed');
        var collapsed = pluginSidebar.classList.contains('collapsed');
        if (pluginIcon) pluginIcon.name = collapsed ? 'chevron-right' : 'chevron-left';
        localStorage.setItem('tbd-plugin-sidebar-collapsed', collapsed ? '1' : '0');
      });
    }

    // Preset sidebar removed — presets managed in-slot only
  }

  // ─── Initialization ─────────────────────────────────────

  function setupRemoveButtons() {
    for (var ch = 0; ch < 2; ch++) {
      (function(ch) {
        var btn = document.getElementById('remove-plugin-' + ch);
        if (btn) {
          btn.addEventListener('click', function() {
            if (state.activePlugin[ch]) {
              clearSlot(ch);
            }
          });
        }
      })(ch);
    }
  }

  async function init() {
    if (state.initialized) {
      // Refresh data on reconnect
      await refreshAll();
      return;
    }

    // getPlugins is critical — let it throw so app.js can detect failure
    // and trigger the reconnection monitor.
    state.plugins = await S.queuedFetch('/plugins?action=list') || [];

    // Fetch IO capabilities (CV/TRIG sources) — non-critical, tolerate failure
    try {
      state.ioCaps = await S.queuedFetch('/device?action=getIOCaps');
    } catch (e) {
      console.warn('Failed to load IO capabilities:', e);
      state.ioCaps = null;
    }

    renderPluginList();
    setupPluginListEvents();
    setupPresetEvents();
    setupSavePresetDialog();
    setupFavoritesBar();
    setupRemoveButtons();
    setupSidebarToggles();
    setupSwapSlots();
    setupSlotSelectDialog();

    // Load slots sequentially — ESP32 httpd has limited sockets,
    // so we avoid concurrent API requests during init.
    // Individual slot failures are tolerated (loadSlotData catches internally).
    await loadSlotData(0);
    await loadSlotData(1);
    renderPluginList(); // re-render with active indicators

    state.initialized = true;
  }

  async function refreshAll() {
    try {
      await loadSlotData(0);
      await loadSlotData(1);
      renderPluginList();
    } catch (e) {
      console.error('Refresh failed:', e);
    }
  }

  /**
   * Re-render parameter UI for both slots without refetching data.
   * Called when display mode toggles (Config ↔ Control).
   */
  function rerenderParams() {
    for (var ch = 0; ch < 2; ch++) {
      if (state.params[ch]) {
        renderParams(ch);
      }
    }
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.pluginManager = {
    init: init,
    state: state,
    favoritesCache: null,
    rerenderParams: rerenderParams,
  };

  // Keep reference updated when cache loads
  var origLoadCache = loadFavoritesCache;
  loadFavoritesCache = async function() {
    await origLoadCache();
    window.TBD.pluginManager.favoritesCache = favoritesCache;
  };

})();

// ── sample-manager.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 Sample Manager  —  Client Application
// Vanilla JS  ·  Shoelace Web Components  ·  Sortable.js
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

// ─── Shared Utilities (from shared.js when available) ────────
var _S = (window.TBD && window.TBD.shared) ? window.TBD.shared : null;

// ─── Configuration ───────────────────────────────────────────
const API_BASE      = '/api/v2/samples';
const SAMPLE_RATE   = 44100;
const MAX_FILENAME  = 32;
const SLICES_PER_BANK = 32;
const NUM_BANKS     = 8;
const PSRAM_MAX     = 29_360_128;           // ~28 MB
const UPLOAD_SOFT_LIMIT = 10 * 1024 * 1024; // 10 MB
const USER_FOLDER   = 'my-samples';         // writable user area

const DEFAULT_BANKS = [
  { name: 'KICK',     color: '#4CAF50' },
  { name: 'SNARE',    color: '#2196F3' },
  { name: 'HIHAT CL', color: '#FFC107' },
  { name: 'HIHAT OP', color: '#FF9800' },
  { name: 'CLAP',     color: '#E91E63' },
  { name: 'RIM',      color: '#9C27B0' },
  { name: 'PERC',     color: '#00BCD4' },
  { name: 'OTHER',    color: '#607D8B' },
];

// Extra color palette for banks beyond the defaults
const BANK_COLORS = [
  '#4CAF50', '#2196F3', '#FFC107', '#FF9800',
  '#E91E63', '#9C27B0', '#00BCD4', '#607D8B',
  '#8BC34A', '#03A9F4', '#FFEB3B', '#FF5722',
  '#673AB7', '#009688', '#795548', '#F44336',
];

// ─── Application State ──────────────────────────────────────
const state = {
  files: [],            // [{ name, path, size }] from API
  folders: [],          // unique folder paths

  kits: {               // from sample_rom.jsn via API
    smp_banks: [],
    smp_bank_names: [],
    smp_bank_tags: [],
    smp_bank_meta: [],
    active_smp_bank: 0,
  },

  kitEntries: [],       // flat Kit descriptor array (may have nulls)
  banks: DEFAULT_BANKS.map(b => ({ ...b, collapsed: false })),

  capacity: { psram_max_bytes: PSRAM_MAX, active_bank_bytes: 0 },

  viewMode: 'banked',   // 'banked' | 'flat'
  targetFolder: USER_FOLDER,
  targetBank: -1,

  // Pool navigation — current folder being viewed ("" = root)
  poolPath: '',
  poolSort: { key: 'name', dir: 'asc' },  // key: 'name'|'dur'|'smp', dir: 'asc'|'desc'

  uploadQueue: [],
  uploading: false,

  audioCtx: null,
  currentSource: null,

  _renameCtx: null,
  _deleteCtx: null,
  _pickerBank: -1,
  _pickerSelected: new Set(),

  // Multi-select for batch operations
  selectedFiles: new Set(),   // Set of 'path/name' keys
  selectionMode: false,

  sortableInstances: [],
  initializing: true,
  dirty: false,
};

let uploadIdCounter = 0;

// ═══════════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════════

function formatDuration(nsamples) {
  const s = nsamples / SAMPLE_RATE;
  return s < 1 ? `${(s * 1000) | 0}ms` : `${s.toFixed(2)}s`;
}

function formatBytes(bytes) {
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
  if (bytes < 1073741824) return `${(bytes / 1048576).toFixed(1)} MB`;
  return `${(bytes / 1073741824).toFixed(1)} GB`;
}

function fileDuration(size) {
  return formatDuration(Math.max(0, (size - 44) / 2));
}

function nsamples(size) { return Math.max(0, (size - 44) / 2); }

function formatSamples(size) {
  const n = nsamples(size);
  if (n >= 1_000_000) return (n / 1_000_000).toFixed(1) + 'M';
  if (n >= 1_000) return (n / 1_000).toFixed(1) + 'k';
  return String(Math.round(n));
}

function formatDate(ms) {
  if (!ms) return '';
  // FAT32 on ESP32 without RTC returns epoch 1980-01-01. Dates before
  // 2020 are meaningless — hide them rather than showing "01/01/80".
  const d = new Date(ms);
  if (d.getFullYear() < 2020) return '';
  const mo = String(d.getMonth() + 1).padStart(2, '0');
  const da = String(d.getDate()).padStart(2, '0');
  const yr = String(d.getFullYear()).slice(-2);
  const hh = String(d.getHours()).padStart(2, '0');
  const mm = String(d.getMinutes()).padStart(2, '0');
  return `${mo}/${da}/${yr} ${hh}:${mm}`;
}

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}

/** Check if a path is inside the user-writable folder */
function isUserWritable(path) {
  if (!path) return false;
  return path === USER_FOLDER || path.startsWith(USER_FOLDER + '/');
}

/** Check if a path is a CHILD of the user folder (not the folder itself) */
function isUserWritableChild(path) {
  if (!path) return false;
  return path.startsWith(USER_FOLDER + '/');
}

/** Check if we're currently browsing inside the user-writable folder */
function isInUserFolder() {
  return isUserWritable(state.poolPath);
}

// ═══════════════════════════════════════════════════════════════
//  WAV ENCODING & CONVERSION
// ═══════════════════════════════════════════════════════════════

function writeString(view, off, str) {
  for (let i = 0; i < str.length; i++) view.setUint8(off + i, str.charCodeAt(i));
}

function encodeWAV(samples, sr = SAMPLE_RATE) {
  const bps = 16, nCh = 1;
  const blockAlign = nCh * (bps / 8);
  const dataBytes = samples.length * (bps / 8);
  const total = 44 + dataBytes;
  const buf = new ArrayBuffer(total);
  const v = new DataView(buf);

  writeString(v, 0, 'RIFF');
  v.setUint32(4, total - 8, true);
  writeString(v, 8, 'WAVE');
  writeString(v, 12, 'fmt ');
  v.setUint32(16, 16, true);
  v.setUint16(20, 1, true);
  v.setUint16(22, nCh, true);
  v.setUint32(24, sr, true);
  v.setUint32(28, sr * blockAlign, true);
  v.setUint16(32, blockAlign, true);
  v.setUint16(34, bps, true);
  writeString(v, 36, 'data');
  v.setUint32(40, dataBytes, true);

  let off = 44;
  for (let i = 0; i < samples.length; i++) {
    const s = Math.max(-1, Math.min(1, samples[i]));
    v.setInt16(off, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
    off += 2;
  }
  return new Blob([buf], { type: 'audio/wav' });
}

function sanitizeFilename(name, maxLen = MAX_FILENAME) {
  let stem = name.replace(/\.[^.]+$/, '');
  stem = stem.normalize('NFKD').replace(/[^\x00-\x7F]/g, '');
  stem = stem.replace(/[\s\-]+/g, '_');
  stem = stem.replace(/[^A-Za-z0-9_]/g, '');
  stem = stem.replace(/_+/g, '_').replace(/^_|_$/g, '');
  if (!stem) stem = 'SAMPLE';
  if (stem.length > maxLen) {
    const half = Math.floor((maxLen - 1) / 2);
    stem = stem.slice(0, half) + '_' + stem.slice(-(maxLen - 1 - half));
  }
  return stem;
}

async function convertToWAV(file) {
  if (!state.audioCtx) {
    state.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
  const arrayBuf = await file.arrayBuffer();
  const audioBuf = await state.audioCtx.decodeAudioData(arrayBuf);
  const numFrames = Math.ceil(audioBuf.duration * SAMPLE_RATE);
  const offCtx = new OfflineAudioContext(1, numFrames, SAMPLE_RATE);
  const src = offCtx.createBufferSource();
  src.buffer = audioBuf;
  src.connect(offCtx.destination);
  src.start(0);
  const rendered = await offCtx.startRendering();
  const mono = rendered.getChannelData(0);
  const blob = encodeWAV(mono, SAMPLE_RATE);
  return {
    blob,
    nsamples: mono.length,
    duration: mono.length / SAMPLE_RATE,
    byteLength: blob.size,
  };
}

// ═══════════════════════════════════════════════════════════════
//  API CLIENT — routes through global queue + safe JSON parsing
// ═══════════════════════════════════════════════════════════════

var _apiTimeout = (_S && _S.API_TIMEOUT_MS) ? _S.API_TIMEOUT_MS : 5000;
var _apiQueue   = (_S && _S.apiQueue)       ? _S.apiQueue       : null;
// Longer timeout for sample list fetch — scanning hundreds of files on SD card
// can take 15-30s on FAT32 with slow I/O.
var _sampleListTimeout = 45000;

async function _rawApiGet(queryString, timeoutMs) {
  const r = await fetch(`${API_BASE}${queryString}`, {
    signal: AbortSignal.timeout(timeoutMs || _apiTimeout),
  });
  if (!r.ok) throw new Error(`API ${r.status}`);
  var text = await r.text();
  if (!text || !text.trim()) return {};
  try { return JSON.parse(text); } catch(e) { return {}; }
}

async function _rawApiPost(queryString, body) {
  const r = await fetch(`${API_BASE}${queryString}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
    signal: AbortSignal.timeout(_apiTimeout),
  });
  if (!r.ok) throw new Error(`API ${r.status}`);
  var text = await r.text();
  if (!text || !text.trim()) return {};
  try { return JSON.parse(text); } catch(e) { return {}; }
}

function apiGet(queryString, timeoutMs) {
  if (_apiQueue) return _apiQueue.enqueue(function() { return _rawApiGet(queryString, timeoutMs); });
  return _rawApiGet(queryString, timeoutMs);
}

function apiPost(queryString, body) {
  if (_apiQueue) return _apiQueue.enqueue(function() { return _rawApiPost(queryString, body); });
  return _rawApiPost(queryString, body);
}

async function fetchSampleList() {
  const kitIdx = state.kits.active_smp_bank || 0;
  const d = await apiGet(`?kit=${kitIdx}`, _sampleListTimeout);
  state.files    = d.files || [];
  state.kits     = d.kits  || state.kits;
  state.kitEntries = d.active_kit_entries || [];
  state.capacity = d.capacity || state.capacity;

  // Load bank metadata
  const meta = state.kits.smp_bank_meta;
  if (meta && meta[state.kits.active_smp_bank] && meta[state.kits.active_smp_bank].banks) {
    const banksMeta = meta[state.kits.active_smp_bank].banks;
    // Rebuild banks array from metadata
    state.banks = banksMeta.map((b, i) => ({
      name: b.name || (DEFAULT_BANKS[i] ? DEFAULT_BANKS[i].name : `BANK ${i + 1}`),
      color: b.color || (DEFAULT_BANKS[i] ? DEFAULT_BANKS[i].color : '#607D8B'),
      collapsed: state.banks[i] ? state.banks[i].collapsed : false,
    }));
  }

  // Derive unique folder list (merge file-derived + firmware-reported directories)
  const fset = new Set(state.files.map(f => f.path));
  // Also include directories reported by firmware (catches empty folders)
  if (d.directories) {
    for (const dir of d.directories) fset.add(dir);
  }
  state.folders = [...fset].filter(Boolean).sort();

  // Mark kit entries whose sample file is missing from the pool
  markMissingKitEntries();
}

/**
 * Mark each kit entry with _missing = true if its sample file
 * cannot be found in state.files (the current sample pool).
 */
function markMissingKitEntries() {
  // Build a Set of "path/filename" keys from available files
  const available = new Set();
  for (const f of state.files) {
    const key = f.path ? `${f.path}/${f.name}` : f.name;
    available.add(key);
  }
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e) continue;
    const key = e.path ? `${e.path}/${e.filename}` : e.filename;
    e._missing = !available.has(key);
  }
}

/**
 * Find all kit entries that reference a given file.
 * Returns array of { bankIdx, slotIdx, entry } for display in warnings.
 */
function findKitReferencesForFile(path, filename) {
  const refs = [];
  if (!state.kitEntries) return refs;
  const targetKey = path ? `${path}/${filename}` : filename;
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e) continue;
    const eKey = e.path ? `${e.path}/${e.filename}` : e.filename;
    if (eKey === targetKey) {
      const bankIdx = Math.floor(i / SLICES_PER_BANK);
      const slotIdx = i - bankIdx * SLICES_PER_BANK;
      refs.push({ bankIdx, slotIdx, entry: e });
    }
  }
  return refs;
}

function uploadSample(blob, path, filename, onProgress) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    const url = `${API_BASE}?action=upload&path=${encodeURIComponent(path)}&filename=${encodeURIComponent(filename)}`;
    xhr.open('POST', url);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    xhr.upload.onprogress = e => {
      if (e.lengthComputable && onProgress) onProgress(Math.round(e.loaded / e.total * 100));
    };
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        try { resolve(JSON.parse(xhr.responseText)); }
        catch { resolve({ ok: true }); }
      } else { reject(new Error(`Upload ${xhr.status}`)); }
    };
    xhr.onerror = () => reject(new Error('Upload network error'));
    xhr.send(blob);
  });
}

async function renameSample(path, oldName, newName) {
  return apiPost('?action=manage', { action: 'rename', path, oldName, newName });
}

async function deleteSample(path, filename) {
  return apiPost('?action=manage', { action: 'delete', path, filename });
}

async function saveKitDescriptor(entries, banksMeta) {
  return apiPost('?action=manage', {
    action: 'saveKit',
    bankIndex: state.kits.active_smp_bank,
    entries,
    banksMeta,
  });
}

async function reloadPSRAM() {
  return apiPost('?action=reload', {});
}

async function createKitOnDevice(name, entries) {
  return apiPost('?action=manage', { action: 'createKit', name, entries });
}

async function createFolderOnDevice(folderPath) {
  return apiPost('?action=manage', { action: 'createFolder', path: folderPath });
}

async function deleteKitOnDevice(kitIndex) {
  return apiPost('?action=manage', { action: 'deleteKit', kitIndex });
}

async function renameFolderOnDevice(oldPath, newPath) {
  return apiPost('?action=manage', { action: 'renameFolder', oldPath, newPath });
}

async function deleteFolderOnDevice(folderPath) {
  return apiPost('?action=manage', { action: 'deleteFolder', path: folderPath });
}

// ═══════════════════════════════════════════════════════════════
//  KIT SAFETY — Check if folder operations affect any kit
// ═══════════════════════════════════════════════════════════════

/**
 * Find all kit entries that reference files within a given folder path.
 * Returns an array of { kitIndex, entryIndex, entry } objects.
 */
function findKitReferencesInFolder(folderPath) {
  const refs = [];
  if (!state.kitEntries) return refs;
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e || !e.path) continue;
    // Match exact folder or subfolder
    if (e.path === folderPath || e.path.startsWith(folderPath + '/')) {
      refs.push({ kitIndex: state.kits.active_smp_bank, entryIndex: i, entry: e });
    }
  }
  return refs;
}

/**
 * Update kit entries in-memory after a folder rename.
 * Changes all path references from oldPath to newPath.
 * Returns the number of entries updated.
 */
function updateKitPathsAfterRename(oldPath, newPath) {
  let count = 0;
  if (!state.kitEntries) return count;
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e || !e.path) continue;
    if (e.path === oldPath) {
      e.path = newPath;
      count++;
    } else if (e.path.startsWith(oldPath + '/')) {
      e.path = newPath + e.path.substring(oldPath.length);
      count++;
    }
  }
  return count;
}

// ═══════════════════════════════════════════════════════════════
//  UPLOAD QUEUE
// ═══════════════════════════════════════════════════════════════

function addToUploadQueue(fileList, targetPath, targetBank) {
  for (const file of fileList) {
    let name = sanitizeFilename(file.name);

    // De-duplicate
    let counter = 2;
    const taken = n =>
      state.files.some(f => f.name === n && f.path === targetPath) ||
      state.uploadQueue.some(q => q.sanitizedName === n && q.targetPath === targetPath && q.status !== 'error');
    while (taken(name)) {
      const suffix = '_' + counter++;
      const base = sanitizeFilename(file.name, MAX_FILENAME - suffix.length);
      name = base + suffix;
    }

    state.uploadQueue.push({
      id: ++uploadIdCounter,
      file,
      originalName: file.name,
      sanitizedName: name,
      targetPath,
      targetBank,
      status: 'queued',
      progress: 0,
      error: null,
    });
  }

  renderTransferBar();
  processUploadQueue();
}

async function processUploadQueue() {
  if (state.uploading) return;
  state.uploading = true;

  while (true) {
    const item = state.uploadQueue.find(q => q.status === 'queued');
    if (!item) break;

    try {
      item.status = 'converting';
      renderTransferBar();

      const result = await convertToWAV(item.file);

      if (result.byteLength > UPLOAD_SOFT_LIMIT) {
        toast(`${item.originalName}: large file (${formatBytes(result.byteLength)})`, 'warning');
      }

      item.status = 'uploading';
      item.byteLength = result.byteLength;
      renderTransferBar();

      await uploadSample(result.blob, item.targetPath, item.sanitizedName, pct => {
        item.progress = pct;
        renderTransferBar();
      });

      item.status = 'done';
      item.nsamples = result.nsamples;
      item.duration = result.duration;

      if (item.targetBank >= 0) {
        addEntryToBank(item.targetBank, item.sanitizedName, item.targetPath, result.nsamples);
      }

      // No per-file toast — the transfer panel provides sufficient feedback
    } catch (e) {
      item.status = 'error';
      item.error = e.message;
      toast(`Failed: ${item.originalName} — ${e.message}`, 'danger');
    }
    renderTransferBar();
  }

  state.uploading = false;

  // Refresh after batch
  try {
    await fetchSampleList();
    renderPoolContent();
    renderKitEditor();
    updateCapacityBar();
    updateStorageBar();
  } catch (e) {
    console.error('Refresh after upload failed:', e);
  }
}

// ═══════════════════════════════════════════════════════════════
//  AUDIO PREVIEW
// ═══════════════════════════════════════════════════════════════

async function playPreview(path, filename) {
  stopPreview();
  if (!state.audioCtx) {
    state.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
  try {
    const url = `${API_BASE}?preview=${encodeURIComponent(path + '/' + filename)}`;
    const r = await (_apiQueue ? _apiQueue.enqueue(function() {
      return fetch(url, { signal: AbortSignal.timeout(_apiTimeout) });
    }) : fetch(url, { signal: AbortSignal.timeout(_apiTimeout) }));
    if (!r.ok) throw new Error(`Preview ${r.status}`);
    const ab = await r.arrayBuffer();
    const buf = await state.audioCtx.decodeAudioData(ab);
    const src = state.audioCtx.createBufferSource();
    src.buffer = buf;
    src.connect(state.audioCtx.destination);
    src.onended = () => { state.currentSource = null; };
    src.start(0);
    state.currentSource = src;
  } catch (e) {
    console.error('Preview error:', e);
    toast('Preview failed', 'danger');
  }
}

function stopPreview() {
  if (state.currentSource) {
    try { state.currentSource.stop(); } catch { /* ignore */ }
    state.currentSource = null;
  }
}

// ═══════════════════════════════════════════════════════════════
//  KIT / BANK LOGIC
// ═══════════════════════════════════════════════════════════════

function getBankEntries(bankIdx) {
  const start = bankIdx * SLICES_PER_BANK;
  const end   = start + SLICES_PER_BANK;
  const out = [];
  for (let i = start; i < end; i++) {
    if (i < state.kitEntries.length && state.kitEntries[i]) {
      out.push({ ...state.kitEntries[i], _idx: i });
    }
  }
  return out;
}

function addEntryToBank(bankIdx, filename, path, nsamp) {
  const start = bankIdx * SLICES_PER_BANK;
  const entries = getBankEntries(bankIdx);
  if (entries.length >= SLICES_PER_BANK) {
    toast(`Bank ${state.banks[bankIdx].name} is full`, 'warning');
    return false;
  }
  const pos = start + entries.length;
  while (state.kitEntries.length <= pos) state.kitEntries.push(null);
  state.kitEntries[pos] = { filename, path, nsamples: nsamp, sname: '' };
  return true;
}

function removeEntryFromBank(absIdx) {
  if (absIdx >= 0 && absIdx < state.kitEntries.length) {
    state.kitEntries[absIdx] = null;
    compactBank(Math.floor(absIdx / SLICES_PER_BANK));
  }
}

function compactBank(bankIdx) {
  const start = bankIdx * SLICES_PER_BANK;
  const entries = [];
  for (let i = start; i < start + SLICES_PER_BANK && i < state.kitEntries.length; i++) {
    if (state.kitEntries[i]) entries.push(state.kitEntries[i]);
  }
  while (state.kitEntries.length < start + SLICES_PER_BANK) state.kitEntries.push(null);
  for (let i = 0; i < SLICES_PER_BANK; i++) {
    state.kitEntries[start + i] = i < entries.length ? entries[i] : null;
  }
}

function reorderBankSlots(bankIdx, oldSlot, newSlot) {
  const start = bankIdx * SLICES_PER_BANK;
  const entries = getBankEntries(bankIdx).map(e => {
    const { _idx, ...rest } = e; return rest;
  });
  const [moved] = entries.splice(oldSlot, 1);
  entries.splice(newSlot, 0, moved);
  while (state.kitEntries.length < start + SLICES_PER_BANK) state.kitEntries.push(null);
  for (let i = 0; i < SLICES_PER_BANK; i++) {
    state.kitEntries[start + i] = i < entries.length ? entries[i] : null;
  }
}

function getKitForSave() {
  // Preserve null entries so bank slot positions are maintained.
  // Without this, a dense array would put all samples in bank 0 on reload.
  const maxIdx = state.banks.length * SLICES_PER_BANK;
  const result = [];
  for (let i = 0; i < maxIdx; i++) {
    if (i < state.kitEntries.length && state.kitEntries[i]) {
      result.push(state.kitEntries[i]);
    } else {
      result.push(null);
    }
  }
  // Trim trailing nulls for storage efficiency
  while (result.length > 0 && result[result.length - 1] === null) {
    result.pop();
  }
  return result;
}

function getBanksMeta() {
  return state.banks.map(b => ({ name: b.name, color: b.color }));
}

function markDirty() {
  state.dirty = true;
  updateSaveButton();
}

function updateSaveButton() {
  const btn = document.getElementById('save-kit-btn');
  if (!btn) return;
  if (state.dirty) {
    btn.variant = 'warning';
    btn.innerHTML = '<sl-icon slot="prefix" name="floppy"></sl-icon>Save *';
  } else {
    btn.variant = 'default';
    btn.innerHTML = '<sl-icon slot="prefix" name="floppy"></sl-icon>Save';
  }
}

async function saveKit() {
  try {
    await saveKitDescriptor(getKitForSave(), getBanksMeta());
    state.dirty = false;
    updateSaveButton();
    toast('Kit saved to SD card. Changes take effect on next power-up.', 'success', 5000);
  } catch (e) {
    toast(`Save failed: ${e.message}`, 'danger');
  }
}



function calculateUsedBytes() {
  let total = 0;
  for (const e of state.kitEntries) {
    if (e && e.nsamples) total += e.nsamples * 2;
  }
  return total;
}

// ─── Bank Management ────────────────────────────────────────

function addBank(name) {
  if (state.banks.length >= 32) {
    toast('Maximum 32 banks reached', 'warning');
    return;
  }
  const idx = state.banks.length;
  const color = BANK_COLORS[idx % BANK_COLORS.length];
  const bankName = name || getNextUniqueBankName();
  state.banks.push({ name: bankName, color, collapsed: false });
  // Extend kitEntries to cover the new bank's slots
  while (state.kitEntries.length < (idx + 1) * SLICES_PER_BANK) {
    state.kitEntries.push(null);
  }
  markDirty();
  renderKitEditor();
  toast(`Bank "${state.banks[idx].name}" added`, 'success');
}

function deleteBank(bankIdx) {
  if (state.banks.length <= 1) {
    toast('Cannot delete the last bank', 'warning');
    return;
  }
  const name = state.banks[bankIdx].name;
  const entries = getBankEntries(bankIdx);
  // Always show confirmation dialog
  openDeleteBankDialog(bankIdx, name, entries.length);
}

function performDeleteBank(bankIdx, name) {
  // Remove the bank's slots from kitEntries
  const start = bankIdx * SLICES_PER_BANK;
  state.kitEntries.splice(start, SLICES_PER_BANK);
  // Remove the bank from state
  state.banks.splice(bankIdx, 1);
  markDirty();
  renderKitEditor();
  toast(`Bank "${name}" deleted`, 'success');
}

/** Generate a unique sequential bank name that doesn't conflict with existing banks */
function getNextUniqueBankName() {
  const existingNames = new Set(state.banks.map(b => b.name));
  let num = state.banks.length + 1;
  while (existingNames.has(`BANK ${num}`)) { num++; }
  return `BANK ${num}`;
}

// ═══════════════════════════════════════════════════════════════
//  TOAST NOTIFICATIONS
//  When bundled with shared.js, toast() and iconForVariant() are
//  already defined with recursion guard (_toastBusy). We use var
//  assignment (not function declaration) so we DON'T overwrite
//  shared.js's guarded version during hoisting.
// ═══════════════════════════════════════════════════════════════

/* eslint-disable no-func-assign */
var iconForVariant = (typeof iconForVariant === 'function') ? iconForVariant : function(v) {
  if (_S && _S.iconForVariant) return _S.iconForVariant(v);
  return { success: 'check2-circle', warning: 'exclamation-triangle',
           danger: 'exclamation-octagon', primary: 'info-circle',
           neutral: 'info-circle' }[v] || 'info-circle';
};

var toast = (typeof toast === 'function') ? toast : function(message, variant, duration) {
  variant = variant || 'primary';
  duration = duration || 4000;
  if (_S && _S.toast) { _S.toast(message, variant, duration); return; }
  var stack = document.getElementById('toast-stack');
  if (!stack) return;
  var alert = document.createElement('sl-alert');
  alert.variant = variant;
  alert.closable = true;
  alert.duration = duration;
  alert.innerHTML = '<sl-icon slot="icon" name="' + iconForVariant(variant) + '"></sl-icon>' + esc(message);
  stack.appendChild(alert);
  customElements.whenDefined('sl-alert').then(function() {
    try { alert.toast(); } catch(e) { console.warn('toast failed:', e); }
  }).catch(function() {});
};

// ═══════════════════════════════════════════════════════════════
//  UI — PSRAM Capacity Bar (Kit Editor)
// ═══════════════════════════════════════════════════════════════

function updateCapacityBar() {
  const used = calculateUsedBytes();
  const max  = state.capacity.psram_max_bytes || PSRAM_MAX;
  const pct  = Math.min(100, Math.round(used / max * 100));
  const fill = document.getElementById('capacity-fill');
  const lbl  = document.getElementById('capacity-label');
  if (fill) {
    fill.style.width = `${pct}%`;
    fill.classList.remove('warn', 'danger');
    if (pct > 90)      fill.classList.add('danger');
    else if (pct > 70) fill.classList.add('warn');
  }
  if (lbl) {
    const usedMB = (used / 1048576).toFixed(1);
    const maxMB  = (max  / 1048576).toFixed(0);
    lbl.textContent = `${usedMB} MB / ${maxMB} MB`;
  }
}

// ═══════════════════════════════════════════════════════════════
//  UI — SD Card Storage Bar (Header)
// ═══════════════════════════════════════════════════════════════

function updateStorageBar() {
  const totalFiles = state.files.length;

  // Update pool file count
  const countEl = document.getElementById('pool-file-count');
  if (countEl) countEl.textContent = `${totalFiles} files`;

  // Update header storage display
  const textEl = document.getElementById('storage-text');
  const fillEl = document.getElementById('storage-bar-fill');
  if (!textEl || !fillEl) return;

  // Use real SD card info from firmware API if available
  const sd = state.capacity;
  const sdTotal = sd.sd_total_bytes || 0;
  const sdFree  = sd.sd_free_bytes  || 0;

  if (sdTotal > 0) {
    const sdUsed  = sdTotal - sdFree;
    const usedPct = Math.min(100, (sdUsed / sdTotal) * 100);
    const freePct = Math.round(100 - usedPct);
    textEl.textContent = `${freePct}% Free / ${formatBytes(sdTotal)}`;
    fillEl.style.width = `${usedPct}%`;
    fillEl.classList.remove('warn', 'danger');
    if (freePct < 10) fillEl.classList.add('danger');
    else if (freePct < 25) fillEl.classList.add('warn');
  } else {
    // Fallback: show file count only, no bar
    const totalBytes = state.files.reduce((sum, f) => sum + (f.size || 0), 0);
    textEl.textContent = `${formatBytes(totalBytes)} used`;
    fillEl.style.width = '0%';
  }
}

// ═══════════════════════════════════════════════════════════════
//  UI — Transfer Bar (bottom, Elektron-style)
// ═══════════════════════════════════════════════════════════════

function renderTransferBar() {
  const bar = document.getElementById('transfer-bar');
  const body = document.getElementById('transfer-body');
  const badge = document.getElementById('transfer-badge');
  const statusEl = document.getElementById('transfer-status');
  const actionsEl = document.getElementById('transfer-actions');

  const all = state.uploadQueue;
  const pending = all.filter(q => q.status === 'queued' || q.status === 'converting' || q.status === 'uploading');
  const done = all.filter(q => q.status === 'done');
  const errors = all.filter(q => q.status === 'error');

  // Badge — show count of pending transfers
  if (pending.length > 0) {
    badge.textContent = pending.length;
    badge.classList.remove('hidden');
    // Auto-expand when there are active transfers
    bar.classList.remove('collapsed');
    bar.classList.add('expanded');
  } else {
    badge.classList.add('hidden');
  }

  // Status text
  if (pending.length > 0) {
    const current = all.find(q => q.status === 'uploading' || q.status === 'converting');
    statusEl.textContent = current
      ? `${current.status === 'converting' ? 'Converting' : 'Uploading'} ${current.sanitizedName}…`
      : `${pending.length} queued`;
  } else if (all.length > 0) {
    statusEl.textContent = `${done.length} done` + (errors.length > 0 ? `, ${errors.length} failed` : '');
  } else {
    statusEl.textContent = '';
  }

  // Clear buttons in header (shown when there are items and nothing pending)
  if (all.length > 0 && pending.length === 0) {
    actionsEl.innerHTML =
      '<button data-act="clear-finished">Clear finished</button>' +
      '<button data-act="clear-all">Clear all</button>';
  } else if (all.length > 0) {
    // During transfers, only allow clear all
    actionsEl.innerHTML = '<button data-act="clear-all">Clear all</button>';
  } else {
    actionsEl.innerHTML = '';
  }

  // Empty state
  if (all.length === 0) {
    body.innerHTML = '<div class="transfer-empty">No transfers yet.</div>';
    return;
  }

  // Table — numbered rows, newest first
  const rows = [...all].reverse().map((item, i) => {
    const num = all.length - i;
    const statusMap = {
      queued: '<span style="color:var(--sl-color-neutral-500);">Waiting</span>',
      converting: '<span style="color:var(--sl-color-primary-600);">Converting…</span>',
      uploading: '<span style="color:var(--sl-color-primary-600);">Uploading</span>',
      done: '<span style="color:var(--sl-color-success-600);">Done</span>',
      error: '<span style="color:var(--sl-color-danger-600);">Failed</span>',
    };
    const statusHTML = statusMap[item.status] || item.status;
    const size = item.byteLength ? formatBytes(item.byteLength) : '—';
    const location = item.targetPath ? '/' + item.targetPath : '';
    const rowClass = item.status === 'done' ? ' class="transfer-row-done"'
      : item.status === 'error' ? ' class="transfer-row-error"' : '';

    let progressHTML = '';
    if (item.status === 'uploading') {
      progressHTML = `<sl-progress-bar value="${item.progress}"></sl-progress-bar>`;
    } else if (item.status === 'converting') {
      progressHTML = '<sl-progress-bar indeterminate></sl-progress-bar>';
    } else if (item.status === 'done') {
      progressHTML = '<sl-progress-bar value="100"></sl-progress-bar>';
    }

    return `<tr${rowClass}>
      <td class="col-num">${num}</td>
      <td class="col-file" title="${esc(item.originalName)}">${esc(item.sanitizedName)}</td>
      <td class="col-size">${size}</td>
      <td class="col-progress">${progressHTML}</td>
      <td class="col-status">${statusHTML}</td>
      <td class="col-location" title="${esc(location)}">${esc(location)}</td>
    </tr>`;
  }).join('');

  body.innerHTML = `<table class="transfer-table">
    <thead><tr>
      <th>#</th><th>NAME</th><th class="col-r">SIZE</th><th>PROGRESS</th><th>STATUS</th><th>LOCATION</th>
    </tr></thead>
    <tbody>${rows}</tbody>
  </table>`;
}

function clearTransferLog(mode = 'finished') {
  if (mode === 'all') {
    state.uploadQueue = [];
  } else {
    // Only clear completed/error items, keep pending
    state.uploadQueue = state.uploadQueue.filter(
      q => q.status === 'queued' || q.status === 'converting' || q.status === 'uploading'
    );
  }
  renderTransferBar();
}

// ═══════════════════════════════════════════════════════════════
//  MULTI-SELECT & BATCH DELETE
// ═══════════════════════════════════════════════════════════════

function toggleSelectionMode() {
  // Can only use selection mode in user-writable folders
  if (!state.selectionMode && !isInUserFolder()) {
    toast('Select is only available in the my-samples folder', 'warning');
    return;
  }
  state.selectionMode = !state.selectionMode;
  if (!state.selectionMode) {
    state.selectedFiles.clear();
  }
  renderPoolContent();
}

function updateSelectionToolbar() {
  const bar = document.getElementById('selection-toolbar');
  if (!bar) return;
  const writable = isInUserFolder();
  if (state.selectionMode && writable) {
    const count = state.selectedFiles.size;
    bar.classList.add('active');
    bar.innerHTML = `
      <span class="sel-label">${count} selected</span>
      <button class="sel-btn" data-act="select-all">Select All</button>
      <button class="sel-btn" data-act="select-none">Deselect</button>
      <div style="flex:1;"></div>
      <button class="sel-btn ${count > 0 ? 'sel-btn-danger' : ''}" data-act="delete-selected" ${count === 0 ? 'disabled' : ''}>Delete ${count > 0 ? count + ' Files' : 'Selected'}</button>
      <button class="sel-btn" data-act="cancel-select">Cancel</button>`;
  } else {
    bar.classList.remove('active');
    bar.innerHTML = '';
  }
}

function setupSelectionToolbar() {
  const bar = document.getElementById('selection-toolbar');
  if (!bar) return;
  bar.addEventListener('click', e => {
    const btn = e.target.closest('[data-act]');
    if (!btn) return;
    const act = btn.dataset.act;
    if (act === 'select-all') {
      // Select all writable files in current folder
      const items = getPoolItems().filter(i => i.type === 'file' && isUserWritable(i.path));
      for (const item of items) {
        const stem = item.name.replace(/\.wav$/i, '');
        state.selectedFiles.add(`${item.path}/${stem}`);
      }
      renderPoolContent();
    } else if (act === 'select-none') {
      state.selectedFiles.clear();
      renderPoolContent();
    } else if (act === 'delete-selected') {
      openBatchDeleteDialog();
    } else if (act === 'cancel-select') {
      state.selectionMode = false;
      state.selectedFiles.clear();
      renderPoolContent();
    }
  });

  // Handle checkbox changes via delegation on pool content
  document.getElementById('pool-content').addEventListener('change', e => {
    if (e.target.matches('.pool-select-cb')) {
      const key = e.target.dataset.key;
      if (e.target.checked) {
        state.selectedFiles.add(key);
      } else {
        state.selectedFiles.delete(key);
      }
      updateSelectionToolbar();
    }
  });
}

function openBatchDeleteDialog() {
  const count = state.selectedFiles.size;
  if (count === 0) return;
  const dlg = document.getElementById('batch-delete-dialog');
  document.getElementById('batch-delete-msg').textContent =
    `Delete ${count} selected file(s)? This cannot be undone.`;
  dlg.show();
}

function setupBatchDeleteDialog() {
  const dlg = document.getElementById('batch-delete-dialog');
  const ok  = document.getElementById('batch-delete-ok');
  const can = document.getElementById('batch-delete-cancel');

  ok.addEventListener('click', async () => {
    dlg.hide();
    const keys = [...state.selectedFiles];
    let deleted = 0, failed = 0;
    for (const key of keys) {
      const lastSlash = key.lastIndexOf('/');
      const path = key.substring(0, lastSlash);
      const name = key.substring(lastSlash + 1);
      try {
        await deleteSample(path, name);
        deleted++;
      } catch {
        failed++;
      }
    }
    state.selectedFiles.clear();
    state.selectionMode = false;
    toast(`Deleted ${deleted} file(s)${failed > 0 ? `, ${failed} failed` : ''}`,
          failed > 0 ? 'warning' : 'success');
    await fetchSampleList();
    renderPoolContent();
    renderKitEditor();
    updateCapacityBar();
    updateStorageBar();
  });
  can.addEventListener('click', () => dlg.hide());
}

function setupTransferBar() {
  document.getElementById('transfer-bar-header').addEventListener('click', e => {
    // Don't toggle when clicking clear buttons
    if (e.target.closest('.transfer-bar-actions')) return;
    const bar = document.getElementById('transfer-bar');
    bar.classList.toggle('collapsed');
    bar.classList.toggle('expanded');
  });
  // Delegate click for the clear buttons (in header)
  document.getElementById('transfer-actions').addEventListener('click', e => {
    e.stopPropagation();
    const btn = e.target.closest('[data-act]');
    if (!btn) return;
    if (btn.dataset.act === 'clear-all') clearTransferLog('all');
    else if (btn.dataset.act === 'clear-finished') clearTransferLog('finished');
  });
}

// ═══════════════════════════════════════════════════════════════
//  UI — Pool Content (Folder Navigation)
// ═══════════════════════════════════════════════════════════════

/**
 * Get items to display for the current poolPath.
 * At root: show folders + root-level files.
 * Inside a folder: show files in that folder.
 */
function getPoolItems() {
  const currentPath = state.poolPath;
  let folders = [];
  let files = [];

  // Pre-compute folder sizes and file counts recursively under each folder path
  const folderSizeMap = {};
  const folderCountMap = {};
  for (const f of state.files) {
    if (!f.path) continue;
    const parts = f.path.split('/').filter(Boolean);
    let accum = '';
    for (const seg of parts) {
      accum = accum ? accum + '/' + seg : seg;
      folderSizeMap[accum] = (folderSizeMap[accum] || 0) + (f.size || 0);
      folderCountMap[accum] = (folderCountMap[accum] || 0) + 1;
    }
  }

  if (currentPath === '') {
    // Root level: show unique top-level folders + root files
    const topFolders = new Set();
    const rootFiles = [];
    for (const f of state.files) {
      const parts = f.path.split('/').filter(Boolean);
      if (parts.length > 0) {
        topFolders.add(parts[0]);
      }
      if (!f.path || f.path === '/' || f.path === '.') {
        rootFiles.push(f);
      }
    }
    // Also include top-level entries from state.folders (catches empty folders)
    for (const fp of state.folders) {
      const top = fp.split('/').filter(Boolean)[0];
      if (top) topFolders.add(top);
    }
    folders = [...topFolders].sort().map(name => ({
      type: 'folder', name, path: name, size: folderSizeMap[name] || 0, fileCount: folderCountMap[name] || 0,
    }));
    files = rootFiles.map(f => ({ type: 'file', ...f }));
  } else {
    // Inside a folder: show sub-folders and files at this level
    const prefix = currentPath;
    const subFolders = new Set();

    for (const f of state.files) {
      if (f.path === prefix) {
        files.push({ type: 'file', ...f });
      } else if (f.path.startsWith(prefix + '/')) {
        const rest = f.path.substring(prefix.length + 1);
        const nextSeg = rest.split('/')[0];
        if (nextSeg) subFolders.add(nextSeg);
      }
    }
    // Also include sub-folder entries from state.folders (catches empty sub-folders)
    for (const fp of state.folders) {
      if (fp.startsWith(prefix + '/')) {
        const rest = fp.substring(prefix.length + 1);
        const nextSeg = rest.split('/')[0];
        if (nextSeg) subFolders.add(nextSeg);
      }
    }
    folders = [...subFolders].sort().map(name => ({
      type: 'folder', name, path: prefix + '/' + name, size: folderSizeMap[prefix + '/' + name] || 0, fileCount: folderCountMap[prefix + '/' + name] || 0,
    }));
  }

  // Sort files
  const { key, dir } = state.poolSort;
  const mul = dir === 'asc' ? 1 : -1;
  files.sort((a, b) => {
    if (key === 'name') return mul * a.name.localeCompare(b.name);
    if (key === 'dur')  return mul * ((a.size || 0) - (b.size || 0));
    if (key === 'smp')  return mul * ((a.size || 0) - (b.size || 0));
    return 0;
  });

  return [...folders, ...files];
}

function renderBreadcrumb() {
  const el = document.getElementById('pool-breadcrumb');
  const homeHtml = '<sl-icon name="hdd" class="pool-breadcrumb-home" data-nav=""></sl-icon><span class="pool-breadcrumb-home-label" data-nav="">TBD-16 SD Card</span>';
  if (state.poolPath === '') {
    el.innerHTML = homeHtml;
    return;
  }

  const parts = state.poolPath.split('/').filter(Boolean);
  let html = homeHtml;
  for (let i = 0; i < parts.length; i++) {
    const pathSoFar = parts.slice(0, i + 1).join('/');
    html += '<span class="pool-breadcrumb-sep">/</span>';
    if (i < parts.length - 1) {
      html += `<span class="pool-breadcrumb-seg" data-nav="${esc(pathSoFar)}">${esc(parts[i])}</span>`;
    } else {
      html += `<span class="pool-breadcrumb-current">${esc(parts[i])}</span>`;
    }
  }
  el.innerHTML = html;
}

function renderPoolContent() {
  const wrap = document.getElementById('pool-content');
  const items = getPoolItems();

  renderBreadcrumb();
  updateSelectionToolbar();

  if (items.length === 0) {
    wrap.innerHTML = '<div style="color:var(--sl-color-neutral-400);font-size:0.8rem;padding:1rem;text-align:center;">Empty folder.</div>';
    updateStorageBar();
    return;
  }

  const inUserArea = isInUserFolder();

  wrap.innerHTML = items.map(item => {
    if (item.type === 'folder') {
      // Show rename/delete only for sub-folders inside user folder, NOT for my-samples itself
      const folderEditable = isUserWritableChild(item.path);
      const folderActions = folderEditable
        ? `<sl-icon-button name="pencil" label="Rename folder" class="action-hover" data-act="rename-folder" data-folder-path="${esc(item.path)}" data-folder-name="${esc(item.name)}" onclick="event.stopPropagation();"></sl-icon-button>
           <sl-icon-button name="trash3" label="Delete folder" class="action-hover" data-act="delete-folder" data-folder-path="${esc(item.path)}" data-folder-name="${esc(item.name)}" onclick="event.stopPropagation();"></sl-icon-button>`
        : '';
      const countLabel = item.fileCount ? `${item.fileCount} sample${item.fileCount !== 1 ? 's' : ''}` : '';
      return `<div class="sample-row folder-row" data-nav="${esc(item.path)}" data-folder-path="${esc(item.path)}" draggable="true">
        <sl-icon name="folder2-open" class="sample-row-icon"></sl-icon>
        <span class="sample-row-name">${esc(item.name)}</span>
        <span class="sample-row-dur" style="opacity:0.5;font-size:0.72rem;">${countLabel}</span>
        <span class="sample-row-smp" style="opacity:0.6">${item.size ? formatBytes(item.size) : ''}</span>
        <div class="sample-row-actions">
          ${folderActions}
          <sl-icon-button name="chevron-right" label="Open" style="font-size:0.85rem;color:var(--sl-color-neutral-400);"></sl-icon-button>
        </div>
      </div>`;
    }

    const stem = item.name.replace(/\.wav$/i, '');
    const durStr = item.size ? fileDuration(item.size) : '';
    const fileWritable = isUserWritable(item.path);
    const fileKey = `${item.path}/${stem}`;
    const isSelected = state.selectedFiles.has(fileKey);

    // Checkbox for multi-select (only in user-writable areas)
    const checkboxHTML = (state.selectionMode && fileWritable)
      ? `<input type="checkbox" class="pool-select-cb" data-key="${esc(fileKey)}" ${isSelected ? 'checked' : ''} onclick="event.stopPropagation();">`
      : '';

    const editActions = fileWritable
      ? `<sl-icon-button name="pencil"     label="Rename" class="action-hover" data-act="rename"   data-path="${esc(item.path)}" data-name="${esc(stem)}"></sl-icon-button>
         <sl-icon-button name="trash3"     label="Delete" class="action-hover" data-act="delete"   data-path="${esc(item.path)}" data-name="${esc(stem)}"></sl-icon-button>`
      : '';
    return `<div class="sample-row file-row pool-file${isSelected ? ' selected' : ''}"
        data-path="${esc(item.path)}" data-name="${esc(stem)}" data-size="${item.size}"
        draggable="true">
      ${checkboxHTML}
      <sl-icon name="music-note-beamed" class="sample-row-icon"></sl-icon>
      <span class="sample-row-name" title="${esc(stem)}">${esc(stem)}</span>
      <span class="sample-row-dur">${durStr}</span>
      <span class="sample-row-smp">${item.size ? formatBytes(item.size) : ''}</span>
      <div class="sample-row-actions">
        <sl-icon-button name="play-fill"  label="Preview" data-act="preview"  data-path="${esc(item.path)}" data-name="${esc(stem)}"></sl-icon-button>
        ${editActions}
      </div>
    </div>`;
  }).join('');

  updateSortArrows();
  updateStorageBar();
}

function navigatePool(path) {
  state.poolPath = path;
  renderPoolContent();
  updateDropZoneTarget();
}

function updateSortArrows() {
  const header = document.getElementById('pool-col-header');
  if (!header) return;
  header.querySelectorAll('.col-sortable').forEach(el => {
    const arrow = el.querySelector('.sort-arrow');
    const key = el.dataset.sort;
    if (key === state.poolSort.key) {
      arrow.textContent = state.poolSort.dir === 'asc' ? '\u25B2' : '\u25BC';
    } else {
      arrow.textContent = '';
    }
  });
}

function setupColumnSort() {
  const header = document.getElementById('pool-col-header');
  if (!header) return;
  header.addEventListener('click', e => {
    const col = e.target.closest('.col-sortable');
    if (!col) return;
    const key = col.dataset.sort;
    if (state.poolSort.key === key) {
      state.poolSort.dir = state.poolSort.dir === 'asc' ? 'desc' : 'asc';
    } else {
      state.poolSort.key = key;
      state.poolSort.dir = 'asc';
    }
    renderPoolContent();
  });
}

// ═══════════════════════════════════════════════════════════════
//  UI — Kit Selector
// ═══════════════════════════════════════════════════════════════

function renderKitSelector() {
  const sel = document.getElementById('kit-select');
  const names = state.kits.smp_bank_names || [];
  const activeIdx = state.kits.active_smp_bank || 0;
  if (names.length === 0) {
    sel.innerHTML = '<sl-option value="0">Default</sl-option>';
    customElements.whenDefined('sl-select').then(() => {
      requestAnimationFrame(() => { sel.value = '0'; });
    });
    return;
  }
  sel.innerHTML = names.map((n, i) =>
    `<sl-option value="${i}">${esc(n)}${i === activeIdx ? ' \u25CF' : ''}</sl-option>`
  ).join('');
  const kitVal = String(activeIdx);
  customElements.whenDefined('sl-select').then(() => {
    requestAnimationFrame(() => { sel.value = kitVal; });
  });
  // Update PSRAM active label
  const label = document.getElementById('psram-active-label');
  if (label) {
    const activeName = names[activeIdx] || 'Default';
    label.textContent = `PSRAM: ${activeName}`;
    label.title = `Kit "${activeName}" is loaded in PSRAM`;
  }
}

// ═══════════════════════════════════════════════════════════════
//  UI — Kit Editor (Right Panel)
// ═══════════════════════════════════════════════════════════════

function renderKitEditor() {
  renderKitSelector();
  const container = document.getElementById('bank-container');
  if (state.viewMode === 'banked') {
    renderBankedView(container);
  } else {
    renderFlatView(container);
  }
  setupBankSortables();
  updateCapacityBar();
}

function renderBankedView(container) {
  let html = '';
  for (let i = 0; i < state.banks.length; i++) {
    const bank = state.banks[i];
    const entries = getBankEntries(i);
    const cls = bank.collapsed ? ' collapsed' : '';
    const displayIdx = String(i + 1).padStart(2, '0');
    html += `<div class="bank-card${cls}" data-bank="${i}">
      <div class="bank-header" data-act="toggle-bank" data-bank="${i}"
           style="background:color-mix(in srgb, ${bank.color} 12%, transparent);border-left:3px solid ${bank.color};">
        <div class="bank-header-left">
          <sl-icon name="chevron-down" class="bank-chevron"></sl-icon>
          <span class="bank-idx">${displayIdx}</span>
          <input class="bank-name-input" value="${esc(bank.name)}"
                 data-act="edit-bank-name" data-bank="${i}" spellcheck="false"
                 onclick="event.stopPropagation();">
          <span class="bank-count">${entries.length}/${SLICES_PER_BANK}</span>
        </div>
        <div class="bank-header-right">
          <sl-icon-button name="trash3" label="Delete bank" class="bank-delete-btn"
                         data-act="delete-bank" data-bank="${i}"></sl-icon-button>
        </div>
      </div>
      <div class="bank-body" data-bank="${i}" data-drop="bank">
        ${entries.length === 0
          ? '<div class="bank-empty">Empty — drop samples here</div>'
          : renderBankSlots(entries, i)}
        <sl-button class="bank-add-btn" variant="text" size="small"
                   data-act="add-sample" data-bank="${i}">
          <sl-icon slot="prefix" name="plus-lg"></sl-icon>Add sample
        </sl-button>
      </div>
    </div>`;
  }
  html += `<button class="add-bank-btn" data-act="add-bank">
    <sl-icon name="plus-circle"></sl-icon> Add Bank
  </button>`;
  container.innerHTML = html;
}

function renderFlatView(container) {
  const allEntries = [];
  for (let i = 0; i < state.kitEntries.length; i++) {
    if (state.kitEntries[i]) allEntries.push({ ...state.kitEntries[i], _idx: i });
  }
  if (allEntries.length === 0) {
    container.innerHTML = '<div class="bank-empty" style="margin-top:1rem;">No samples in this Kit.</div>';
    return;
  }
  let html = '<div class="bank-body" data-bank="0" data-drop="bank">';
  html += allEntries.map(e => {
    const size = e.nsamples ? formatBytes(e.nsamples * 2 + 44) : '?';
    const isMissing = e._missing;
    const missingCls = isMissing ? ' bank-slot-missing' : '';
    const missingIcon = isMissing ? '<sl-icon name="exclamation-triangle" class="bank-slot-missing-icon" title="Sample not found — drag a replacement here"></sl-icon>' : '';
    const previewBtn = isMissing
      ? '<sl-icon-button name="play-fill" label="Preview" disabled style="opacity:0.3;"></sl-icon-button>'
      : `<sl-icon-button name="play-fill" label="Preview" data-act="preview-kit" data-index="${e._idx}"></sl-icon-button>`;
    // Compute bank index & name for this entry
    const bankIdx = Math.floor(e._idx / SLICES_PER_BANK);
    const bank = state.banks[bankIdx];
    const bankName = bank ? bank.name : 'Bank ' + (bankIdx + 1);
    const bankColor = bank ? bank.color : '#888';
    const bankNum = String(bankIdx + 1).padStart(2, '0');
    const bankBadge = `<span class="flat-bank-badge" style="background:color-mix(in srgb, ${bankColor} 15%, transparent);color:${bankColor};border:1px solid color-mix(in srgb, ${bankColor} 30%, transparent);">${esc(bankNum + ' ' + bankName)}</span>`;
    return `<div class="bank-slot${missingCls}" data-index="${e._idx}" data-bank="0" title="${isMissing ? 'Missing sample — drag a file from the pool to replace' : ''}">
      <span class="drag-handle">⫶</span>
      <sl-icon name="music-note-beamed" class="sample-row-icon"></sl-icon>
      <span class="bank-slot-name" title="${esc(e.filename)}">${missingIcon}${esc(e.sname || e.filename)}</span>
      ${bankBadge}
      <span class="bank-slot-dur">${size}</span>
      <div class="bank-slot-actions">
        ${previewBtn}
        <sl-icon-button name="pencil"    label="Edit" class="action-hover" data-act="edit-sname"   data-index="${e._idx}"></sl-icon-button>
        <sl-icon-button name="x-lg"      label="Remove" class="action-hover" data-act="remove-slot"  data-index="${e._idx}"></sl-icon-button>
      </div>
    </div>`;
  }).join('');
  html += '</div>';
  container.innerHTML = html;
}

function renderBankSlots(entries, bankIdx) {
  return entries.map((e, slotIdx) => {
    const size = e.nsamples ? formatBytes(e.nsamples * 2 + 44) : '?';
    const absIdx = bankIdx * SLICES_PER_BANK + slotIdx;
    const isMissing = e._missing;
    const missingCls = isMissing ? ' bank-slot-missing' : '';
    const missingIcon = isMissing ? '<sl-icon name="exclamation-triangle" class="bank-slot-missing-icon" title="Sample not found — drag a replacement here"></sl-icon>' : '';
    const previewBtn = isMissing
      ? '<sl-icon-button name="play-fill" label="Preview" disabled style="opacity:0.3;"></sl-icon-button>'
      : `<sl-icon-button name="play-fill" label="Preview" data-act="preview-kit" data-index="${absIdx}"></sl-icon-button>`;
    return `<div class="bank-slot${missingCls}" data-index="${absIdx}" data-bank="${bankIdx}" title="${isMissing ? 'Missing sample — drag a file from the pool to replace' : ''}">
      <span class="drag-handle">⫶</span>
      <sl-icon name="music-note-beamed" class="sample-row-icon"></sl-icon>
      <span class="bank-slot-name" title="${esc(e.filename)}">${missingIcon}${esc(e.sname || e.filename)}</span>
      <span class="bank-slot-dur">${size}</span>
      <div class="bank-slot-actions">
        ${previewBtn}
        <sl-icon-button name="pencil"    label="Edit" class="action-hover" data-act="edit-sname"   data-index="${absIdx}"></sl-icon-button>
        <sl-icon-button name="x-lg"      label="Remove" class="action-hover" data-act="remove-slot"  data-index="${absIdx}"></sl-icon-button>
      </div>
    </div>`;
  }).join('');
}

// ═══════════════════════════════════════════════════════════════
//  SORTABLE.JS
// ═══════════════════════════════════════════════════════════════

function setupBankSortables() {
  for (const s of state.sortableInstances) { try { s.destroy(); } catch {} }
  state.sortableInstances = [];

  const bodies = document.querySelectorAll('.bank-body');
  for (const body of bodies) {
    const bankIdx = parseInt(body.dataset.bank, 10);
    if (typeof Sortable === 'undefined') break;
    const inst = Sortable.create(body, {
      handle: '.drag-handle',
      animation: 150,
      ghostClass: 'sortable-ghost',
      chosenClass: 'sortable-chosen',
      draggable: '.bank-slot',
      onEnd(evt) {
        if (evt.oldIndex !== evt.newIndex) {
          reorderBankSlots(bankIdx, evt.oldIndex, evt.newIndex);
          markDirty();
          renderKitEditor();
        }
      },
    });
    state.sortableInstances.push(inst);
  }
}

// ═══════════════════════════════════════════════════════════════
//  DRAG & DROP: Pool → Bank
// ═══════════════════════════════════════════════════════════════

function setupPoolDragEvents() {
  const poolPanel = document.getElementById('pool-panel');

  poolPanel.addEventListener('dragstart', e => {
    // Handle folder drag
    const folderRow = e.target.closest('.folder-row[data-folder-path]');
    if (folderRow) {
      const folderPath = folderRow.dataset.folderPath;
      const filesInFolder = state.files.filter(f => f.path === folderPath || f.path.startsWith(folderPath + '/'));
      if (filesInFolder.length === 0) return;
      const samples = filesInFolder.map(f => ({
        name: f.name.replace(/\.wav$/i, ''),
        path: f.path,
        size: f.size || 0,
      }));
      e.dataTransfer.setData('application/x-tbd-samples', JSON.stringify(samples));
      e.dataTransfer.effectAllowed = 'copy';
      return;
    }

    const row = e.target.closest('.pool-file');
    if (!row) return;

    // Multi-select drag: if in selection mode and dragged item is selected, drag all selected
    const draggedKey = `${row.dataset.path}/${row.dataset.name}`;
    if (state.selectionMode && state.selectedFiles.size > 0 && state.selectedFiles.has(draggedKey)) {
      const samples = [];
      for (const key of state.selectedFiles) {
        const lastSlash = key.lastIndexOf('/');
        const path = key.substring(0, lastSlash);
        const name = key.substring(lastSlash + 1);
        const file = state.files.find(f => f.path === path && f.name === name);
        samples.push({ name, path, size: file ? file.size : 0 });
      }
      e.dataTransfer.setData('application/x-tbd-samples', JSON.stringify(samples));
      e.dataTransfer.effectAllowed = 'copy';
      return;
    }

    // Single file drag
    e.dataTransfer.setData('application/x-tbd-sample', JSON.stringify({
      name: row.dataset.name,
      path: row.dataset.path,
      size: parseInt(row.dataset.size, 10),
    }));
    e.dataTransfer.effectAllowed = 'copy';
  });

  const kitPanel = document.getElementById('kit-panel');
  kitPanel.addEventListener('dragover', e => {
    // Allow drop on individual bank slots (replace) or bank body (add)
    const slot = e.target.closest('.bank-slot');
    const body = e.target.closest('[data-drop="bank"]');
    if (!slot && !body) return;
    e.preventDefault();
    e.dataTransfer.dropEffect = 'copy';
    // Only highlight slot for single-sample drops
    const isMulti = e.dataTransfer.types.includes('application/x-tbd-samples');
    if (slot && !isMulti) {
      slot.classList.add('drop-target-slot');
    } else if (body) {
      body.classList.add('drop-target');
    }
  });
  kitPanel.addEventListener('dragleave', e => {
    const slot = e.target.closest('.bank-slot');
    const body = e.target.closest('[data-drop="bank"]');
    if (slot) slot.classList.remove('drop-target-slot');
    if (body) body.classList.remove('drop-target');
  });
  kitPanel.addEventListener('drop', e => {
    // Clear all highlights
    kitPanel.querySelectorAll('.drop-target-slot').forEach(el => el.classList.remove('drop-target-slot'));
    kitPanel.querySelectorAll('.drop-target').forEach(el => el.classList.remove('drop-target'));

    // Handle multi-sample drop (selected files or folder)
    const rawMulti = e.dataTransfer.getData('application/x-tbd-samples');
    if (rawMulti) {
      e.preventDefault();
      try {
        const samples = JSON.parse(rawMulti);
        const body = e.target.closest('[data-drop="bank"]');
        if (!body) return;
        const bankIdx = parseInt(body.dataset.bank, 10);
        let added = 0;
        for (const data of samples) {
          const nsamp = nsamples(data.size);
          if (addEntryToBank(bankIdx, data.name, data.path, nsamp)) added++;
        }
        if (added > 0) {
          markDirty();
          renderKitEditor();
          toast(`Added ${added} sample(s) to ${state.banks[bankIdx].name}`, 'success');
        }
      } catch (err) {
        console.error('Multi-drop parse error:', err);
      }
      return;
    }

    const raw = e.dataTransfer.getData('application/x-tbd-sample');
    if (!raw) return;
    e.preventDefault();

    try {
      const data = JSON.parse(raw);
      const nsamp = nsamples(data.size);

      // Check if dropped onto a specific slot (replace)
      const slot = e.target.closest('.bank-slot');
      if (slot && slot.dataset.index !== undefined) {
        const absIdx = parseInt(slot.dataset.index, 10);
        if (absIdx >= 0 && absIdx < state.kitEntries.length) {
          state.kitEntries[absIdx] = { filename: data.name, path: data.path, nsamples: nsamp, sname: '' };
          markMissingKitEntries();
          markDirty();
          renderKitEditor();
          toast(`Replaced slot with ${data.name}`, 'success');
          return;
        }
      }

      // Otherwise add to bank
      const body = e.target.closest('[data-drop="bank"]');
      if (!body) return;
      const bankIdx = parseInt(body.dataset.bank, 10);
      if (addEntryToBank(bankIdx, data.name, data.path, nsamp)) {
        markDirty();
        renderKitEditor();
        toast(`Added ${data.name} to ${state.banks[bankIdx].name}`, 'success');
      }
    } catch (err) {
      console.error('Drop parse error:', err);
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  DROP ZONE — Drag-and-Drop Folder Helpers
// ═══════════════════════════════════════════════════════════════

/** Read all entries from a FileSystemDirectoryReader (Chrome batches in 100s) */
function readAllDirectoryEntries(dirReader) {
  return new Promise((resolve, reject) => {
    const all = [];
    const readBatch = () => {
      dirReader.readEntries(entries => {
        if (entries.length === 0) resolve(all);
        else { all.push(...entries); readBatch(); }
      }, reject);
    };
    readBatch();
  });
}

/** Get a File object from a FileSystemFileEntry */
function fileFromEntry(entry) {
  return new Promise((resolve, reject) => entry.file(resolve, reject));
}

/** Recursively collect files from FileSystemEntry objects, annotating each with _relativePath */
async function collectFilesFromEntries(entries, relPath, results) {
  for (const entry of entries) {
    if (entry.isFile) {
      try {
        const file = await fileFromEntry(entry);
        file._relativePath = relPath ? relPath + '/' + entry.name : entry.name;
        results.push(file);
      } catch (e) { /* skip unreadable files */ }
    } else if (entry.isDirectory) {
      const subPath = relPath ? relPath + '/' + entry.name : entry.name;
      const reader = entry.createReader();
      const subEntries = await readAllDirectoryEntries(reader);
      await collectFilesFromEntries(subEntries, subPath, results);
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  DROP ZONE — File Upload
// ═══════════════════════════════════════════════════════════════

function setupDropZone() {
  const poolPanel = document.getElementById('pool-panel');
  const overlay = document.getElementById('pool-drop-overlay');
  const bar = document.getElementById('drop-zone-bar');
  const input = document.getElementById('file-input');
  const uploadBtn = document.getElementById('upload-btn');
  let dragCounter = 0;

  // Prevent defaults on whole page
  for (const evt of ['dragenter', 'dragover', 'dragleave', 'drop']) {
    document.body.addEventListener(evt, e => e.preventDefault(), false);
  }

  // Show overlay when dragging files over the pool panel
  poolPanel.addEventListener('dragenter', e => {
    // Only react to external file drags, not internal sample drags
    if (e.dataTransfer.types.includes('Files')) {
      dragCounter++;
      overlay.classList.add('visible');
    }
  });
  poolPanel.addEventListener('dragleave', e => {
    if (e.dataTransfer.types.includes('Files')) {
      dragCounter--;
      if (dragCounter <= 0) {
        dragCounter = 0;
        overlay.classList.remove('visible');
      }
    }
  });
  poolPanel.addEventListener('dragover', e => {
    if (e.dataTransfer.types.includes('Files')) {
      e.preventDefault();
      e.dataTransfer.dropEffect = 'copy';
    }
  });
  poolPanel.addEventListener('drop', async e => {
    dragCounter = 0;
    overlay.classList.remove('visible');
    e.preventDefault();
    e.stopPropagation();

    // Check for dropped folders via File System Entry API
    const items = e.dataTransfer.items ? Array.from(e.dataTransfer.items) : [];
    const entries = items.map(i => i.webkitGetAsEntry ? i.webkitGetAsEntry() : null).filter(Boolean);
    const hasDirs = entries.some(entry => entry && entry.isDirectory);

    if (hasDirs) {
      // Recursively collect files from dropped folders, preserving paths
      const collected = [];
      for (const entry of entries) {
        if (entry.isDirectory) {
          const reader = entry.createReader();
          const children = await readAllDirectoryEntries(reader);
          await collectFilesFromEntries(children, entry.name, collected);
        } else if (entry.isFile) {
          const file = await fileFromEntry(entry);
          file._relativePath = entry.name;
          collected.push(file);
        }
      }
      if (collected.length > 0) handleDroppedFiles(collected);
    } else if (e.dataTransfer.files.length > 0) {
      handleDroppedFiles(e.dataTransfer.files);
    }
  });

  // Bottom bar click and upload button
  const inputFiles = document.getElementById('file-input-files');
  bar.addEventListener('click', () => inputFiles.click());
  uploadBtn.addEventListener('click', e => {
    // Show a context menu with file/folder upload options
    const menu = document.getElementById('upload-menu');
    if (menu) {
      menu.style.display = menu.style.display === 'block' ? 'none' : 'block';
      // Position below the upload button
      const rect = uploadBtn.getBoundingClientRect();
      menu.style.top = (rect.bottom + 2) + 'px';
      menu.style.left = (rect.right - menu.offsetWidth) + 'px';
      e.stopPropagation();
      // Close on outside click
      const closeMenu = () => { menu.style.display = 'none'; document.removeEventListener('click', closeMenu); };
      setTimeout(() => document.addEventListener('click', closeMenu), 0);
    } else {
      inputFiles.click();
    }
  });
  // "Upload Files" option
  const uploadFilesOpt = document.getElementById('upload-files-opt');
  if (uploadFilesOpt) uploadFilesOpt.addEventListener('click', () => { inputFiles.click(); });
  // "Upload Folder" option
  const uploadFolderOpt = document.getElementById('upload-folder-opt');
  if (uploadFolderOpt) uploadFolderOpt.addEventListener('click', () => { input.click(); });
  // File input listeners
  input.addEventListener('change', () => {
    if (input.files.length > 0) {
      handleDroppedFiles(input.files);
      input.value = '';
    }
  });
  inputFiles.addEventListener('change', () => {
    if (inputFiles.files.length > 0) {
      handleDroppedFiles(inputFiles.files);
      inputFiles.value = '';
    }
  });
}

function handleDroppedFiles(fileList) {
  // At root level, reject uploads — user must navigate into a folder first
  if (!state.poolPath) {
    toast('Navigate into a folder before uploading. Uploads go to the currently viewed folder.', 'warning', 5000);
    return;
  }
  const targetPath = state.poolPath;
  // Collect valid audio files from the FileList (supports folder uploads via webkitdirectory)
  const validExts = ['.wav', '.mp3', '.aiff', '.aif', '.ogg', '.flac', '.m4a', '.wma'];
  const validFiles = Array.from(fileList).filter(f => {
    const name = f.name.toLowerCase();
    return validExts.some(ext => name.endsWith(ext)) && f.size > 0;
  });
  if (validFiles.length === 0) {
    toast('No valid audio files found. Supported: WAV, MP3, AIFF, OGG, FLAC', 'warning', 5000);
    return;
  }

  // Check if any files come from subfolders (webkitRelativePath or _relativePath from drag-drop)
  const hasSubfolders = validFiles.some(f => {
    const rel = f.webkitRelativePath || f._relativePath || '';
    const parts = rel.split('/');
    return parts.length > 2; // root-folder / subfolder / file
  });

  if (hasSubfolders) {
    // Auto-preserve folder structure — no confirmation needed
    const subfolderSet = new Set();
    for (const f of validFiles) {
      const rel = f.webkitRelativePath || f._relativePath || '';
      const parts = rel.split('/');
      if (parts.length > 2) subfolderSet.add(parts.slice(1, -1).join('/'));
    }
    toast(`Uploading ${validFiles.length} files — preserving ${subfolderSet.size} subfolder(s)`, 'primary', 4000);
    uploadWithSubfolders(validFiles, targetPath);
  } else {
    addToUploadQueue(validFiles, targetPath, state.targetBank);
  }
}

/**
 * Upload files preserving their subfolder structure.
 * Creates necessary subfolders on the device first, then queues uploads.
 */
async function uploadWithSubfolders(validFiles, basePath) {
  // Collect unique subfolder paths to create
  const subfolderPaths = new Set();
  for (const f of validFiles) {
    const rel = f.webkitRelativePath || f._relativePath || '';
    const parts = rel.split('/');
    if (parts.length > 2) {
      // Build cumulative subfolder paths for recursive creation
      for (let depth = 2; depth < parts.length; depth++) {
        const sub = parts.slice(1, depth).join('/');
        subfolderPaths.add(basePath + '/' + sub);
      }
    }
  }

  // Create subfolders on device (sorted so parents come before children)
  const sortedPaths = Array.from(subfolderPaths).sort();
  for (const folderPath of sortedPaths) {
    try {
      await createFolderOnDevice(folderPath);
    } catch (e) {
      // Folder may already exist — continue
    }
  }

  // Queue each file to its correct subfolder path
  for (const f of validFiles) {
    const rel = f.webkitRelativePath || f._relativePath || '';
    const parts = rel.split('/');
    let uploadPath;
    if (parts.length > 2) {
      uploadPath = basePath + '/' + parts.slice(1, -1).join('/');
    } else {
      uploadPath = basePath;
    }
    addToUploadQueue([f], uploadPath, state.targetBank);
  }

  // Refresh the file list to show new folders
  try {
    await fetchSampleList();
    renderPoolContent();
    updateDropZoneTarget();
  } catch (e) {
    console.error('Refresh after folder upload failed:', e);
  }
}

/**
 * Update the drop-zone bar and overlay text to show the current upload target.
 * Called whenever pool navigation changes.
 */
function updateDropZoneTarget() {
  const barText = document.querySelector('#drop-zone-bar .drop-zone-text');
  const overlayText = document.querySelector('#pool-drop-overlay span');
  const uploadBtn = document.getElementById('upload-btn');
  const newFolderBtn = document.getElementById('new-folder-btn');
  const writable = isInUserFolder();
  if (state.poolPath && writable) {
    const shortPath = '/' + state.poolPath;
    if (barText) barText.textContent = `Drop files to upload → ${shortPath}`;
    if (overlayText) overlayText.textContent = `Drop audio files → ${shortPath}`;
    if (uploadBtn) uploadBtn.removeAttribute('disabled');
  } else if (state.poolPath && !writable) {
    if (barText) barText.textContent = 'Upload to ' + USER_FOLDER + ' folder only';
    if (overlayText) overlayText.textContent = 'Navigate to ' + USER_FOLDER + ' to upload';
    if (uploadBtn) uploadBtn.setAttribute('disabled', '');
  } else {
    if (barText) barText.textContent = 'Navigate into a folder to upload files';
    if (overlayText) overlayText.textContent = 'Navigate into a folder first';
    if (uploadBtn) uploadBtn.setAttribute('disabled', '');
  }
  // Show/hide or disable new-folder button based on writable context
  if (newFolderBtn) {
    // Always show the button, but disable it when not writable
    newFolderBtn.style.display = '';
    if (writable) {
      newFolderBtn.removeAttribute('disabled');
      newFolderBtn.style.opacity = '';
    } else {
      newFolderBtn.setAttribute('disabled', '');
      newFolderBtn.style.opacity = '0.4';
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  EVENT DELEGATION — Pool Actions
// ═══════════════════════════════════════════════════════════════

function setupPoolActions() {
  const pool = document.getElementById('pool-panel');
  pool.addEventListener('click', e => {
    // Handle folder action buttons FIRST (before nav, since they stopPropagation)
    const btn = e.target.closest('[data-act]');
    if (btn) {
      const act = btn.dataset.act;

      if (act === 'rename-folder') {
        e.stopPropagation();
        openRenameFolderDialog(btn.dataset.folderPath, btn.dataset.folderName);
        return;
      }
      if (act === 'delete-folder') {
        e.stopPropagation();
        openDeleteFolderDialog(btn.dataset.folderPath, btn.dataset.folderName);
        return;
      }
      if (act === 'preview') {
        e.stopPropagation();
        playPreview(btn.dataset.path, btn.dataset.name);
        return;
      }
      if (act === 'rename') {
        e.stopPropagation();
        openRenameDialog(btn.dataset.path, btn.dataset.name);
        return;
      }
      if (act === 'delete') {
        e.stopPropagation();
        openDeleteDialog(btn.dataset.path, btn.dataset.name);
        return;
      }
    }

    // Folder navigation via breadcrumb or folder row
    const navEl = e.target.closest('[data-nav]');
    if (navEl) {
      e.stopPropagation();
      navigatePool(navEl.dataset.nav);
      return;
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  EVENT DELEGATION — Kit Editor Actions
// ═══════════════════════════════════════════════════════════════

function setupKitActions() {
  const kit = document.getElementById('kit-panel');
  kit.addEventListener('click', e => {
    const btn = e.target.closest('[data-act]');
    if (!btn) return;
    const act = btn.dataset.act;

    if (act === 'toggle-bank') {
      const bankIdx = parseInt(btn.dataset.bank, 10);
      state.banks[bankIdx].collapsed = !state.banks[bankIdx].collapsed;
      btn.closest('.bank-card').classList.toggle('collapsed');
      return;
    }
    if (act === 'delete-bank') {
      e.stopPropagation();
      deleteBank(parseInt(btn.dataset.bank, 10));
      return;
    }
    if (act === 'add-bank') {
      e.stopPropagation();
      openAddBankDialog();
      return;
    }
    if (act === 'add-sample') {
      e.stopPropagation();
      openSamplePicker(parseInt(btn.dataset.bank, 10));
      return;
    }
    if (act === 'preview-kit') {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      const entry = state.kitEntries[idx];
      if (entry) playPreview(entry.path, entry.filename);
      return;
    }
    if (act === 'edit-sname') {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      const entry = state.kitEntries[idx];
      if (entry) openRenameDisplayName(idx, entry);
      return;
    }
    if (act === 'remove-slot') {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      removeEntryFromBank(idx);
      markDirty();
      renderKitEditor();
      return;
    }
  });

  kit.addEventListener('change', e => {
    if (e.target.matches('[data-act="edit-bank-name"]')) {
      const bankIdx = parseInt(e.target.dataset.bank, 10);
      const fallback = (DEFAULT_BANKS[bankIdx] && DEFAULT_BANKS[bankIdx].name) || `BANK ${bankIdx + 1}`;
      state.banks[bankIdx].name = e.target.value.trim() || fallback;
      markDirty();
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  DIALOGS
// ═══════════════════════════════════════════════════════════════

function openRenameDialog(path, filename) {
  state._renameCtx = { path, filename };
  const dlg = document.getElementById('rename-dialog');
  const inp = document.getElementById('rename-input');
  dlg.label = 'Rename';
  inp.value = filename;
  dlg.show();
  setTimeout(() => inp.focus(), 100);
}

function setupRenameDialog() {
  const dlg = document.getElementById('rename-dialog');
  const ok  = document.getElementById('rename-ok');
  const can = document.getElementById('rename-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._renameCtx;
    if (!ctx) return;
    if (ctx._kitIndex !== undefined) return; // handled by temporary handler
    const newName = sanitizeFilename(document.getElementById('rename-input').value);
    if (!newName || newName === ctx.filename) { dlg.hide(); return; }
    try {
      await renameSample(ctx.path, ctx.filename, newName);
      toast(`Renamed to ${newName}`, 'success');
      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
    } catch (e) {
      toast(`Rename failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

async function openDeleteDialog(path, filename) {
  state._deleteCtx = { path, filename };
  document.getElementById('delete-msg').textContent =
    `Delete ${filename}.wav from ${path}?`;

  // Check ALL kits for references to this file via firmware
  const refsEl = document.getElementById('delete-kit-refs');
  const listEl = document.getElementById('delete-kit-refs-list');
  const headerEl = refsEl.querySelector('div');
  refsEl.style.display = 'none';
  listEl.innerHTML = '';
  document.getElementById('delete-dialog').show();

  try {
    const result = await apiPost('?action=manage', {
      action: 'checkFileRefs', path, filename
    });
    const refs = result.refs || [];
    if (refs.length > 0) {
      const SLICES = SLICES_PER_BANK;
      headerEl.innerHTML = `<sl-icon name="exclamation-triangle" style="vertical-align:-2px;margin-right:4px;color:var(--sl-color-warning-600);"></sl-icon>Used in Kit(s):`;
      listEl.innerHTML = refs.map(r => {
        const bankIdx = Math.floor(r.slotIndex / SLICES);
        const slotInBank = r.slotIndex - bankIdx * SLICES;
        const kitName = r.kitName || `Kit ${r.kitIndex}`;
        return `<li>${kitName} — Bank ${bankIdx + 1}, Slot ${slotInBank + 1}</li>`;
      }).join('');
      refsEl.style.display = '';
    }
  } catch (e) {
    // Non-critical: fall back to local check
    const refs = findKitReferencesForFile(path, filename);
    if (refs.length > 0) {
      headerEl.innerHTML = `<sl-icon name="exclamation-triangle" style="vertical-align:-2px;margin-right:4px;color:var(--sl-color-warning-600);"></sl-icon>Used in current Kit:`;
      listEl.innerHTML = refs.map(r =>
        `<li>${state.banks[r.bankIdx].name} — Slot ${r.slotIdx + 1}</li>`
      ).join('');
      refsEl.style.display = '';
    }
  }
}

function setupDeleteDialog() {
  const dlg = document.getElementById('delete-dialog');
  const ok  = document.getElementById('delete-ok');
  const can = document.getElementById('delete-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._deleteCtx;
    if (!ctx) return;
    try {
      await deleteSample(ctx.path, ctx.filename);
      toast(`Deleted ${ctx.filename}`, 'success');
      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
      updateCapacityBar();
      updateStorageBar();
    } catch (e) {
      toast(`Delete failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  FOLDER MANAGEMENT DIALOGS
// ═══════════════════════════════════════════════════════════════

/**
 * Sanitize a folder name for FAT32 filesystem.
 */
function sanitizeFolderName(name) {
  let clean = name.trim();
  clean = clean.normalize('NFKD').replace(/[^\x00-\x7F]/g, '');
  clean = clean.replace(/[\s]+/g, '_');
  clean = clean.replace(/[^A-Za-z0-9_\-.]/g, '');
  clean = clean.replace(/[_\-.]{2,}/g, '_');
  clean = clean.replace(/^[_\-.]+|[_\-.]+$/g, '');
  if (!clean) clean = 'folder';
  if (clean.length > 32) clean = clean.substring(0, 32);
  return clean;
}

function setupNewFolderDialog() {
  const dlg = document.getElementById('new-folder-dialog');
  const ok  = document.getElementById('new-folder-ok');
  const can = document.getElementById('new-folder-cancel');
  const btn = document.getElementById('new-folder-btn');

  btn.addEventListener('click', () => {
    document.getElementById('new-folder-input').value = '';
    dlg.show();
    setTimeout(() => document.getElementById('new-folder-input').focus(), 100);
  });

  ok.addEventListener('click', async () => {
    const raw = document.getElementById('new-folder-input').value;
    const name = sanitizeFolderName(raw);
    if (!name) { toast('Please enter a folder name.', 'warning'); return; }

    const parentPath = state.poolPath || '';
    const fullPath = parentPath ? `${parentPath}/${name}` : name;

    // Check if folder already exists
    const exists = state.folders.some(f => f === fullPath || f.startsWith(fullPath + '/'));
    if (exists) {
      toast(`Folder "${name}" already exists.`, 'warning');
      return;
    }

    try {
      await createFolderOnDevice(fullPath);
      toast(`Folder "${name}" created.`, 'success');
      await fetchSampleList();
      renderPoolContent();
    } catch (e) {
      toast(`Create folder failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

function openRenameFolderDialog(folderPath, folderName) {
  state._renameFolderCtx = { path: folderPath, name: folderName };
  const dlg = document.getElementById('rename-folder-dialog');
  const inp = document.getElementById('rename-folder-input');
  const warning = document.getElementById('rename-folder-kit-warning');
  const msg = document.getElementById('rename-folder-kit-msg');
  inp.value = folderName;

  // Check kit references
  const refs = findKitReferencesInFolder(folderPath);
  if (refs.length > 0) {
    warning.style.display = '';
    msg.textContent = `${refs.length} sample(s) in the active kit reference this folder. Renaming will automatically update their paths.`;
  } else {
    warning.style.display = 'none';
  }

  dlg.show();
  setTimeout(() => inp.focus(), 100);
}

function setupRenameFolderDialog() {
  const dlg = document.getElementById('rename-folder-dialog');
  const ok  = document.getElementById('rename-folder-ok');
  const can = document.getElementById('rename-folder-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._renameFolderCtx;
    if (!ctx) return;
    const raw = document.getElementById('rename-folder-input').value;
    const newName = sanitizeFolderName(raw);
    if (!newName || newName === ctx.name) { dlg.hide(); return; }

    // Compute new path: replace the last segment
    const parts = ctx.path.split('/');
    parts[parts.length - 1] = newName;
    const newPath = parts.join('/');

    try {
      // Rename on device
      await renameFolderOnDevice(ctx.path, newPath);

      // Update kit entries in memory and auto-save if needed
      const updated = updateKitPathsAfterRename(ctx.path, newPath);
      if (updated > 0) {
        markDirty();
        await saveKit();
        toast(`Renamed folder and updated ${updated} kit reference(s).`, 'success');
      } else {
        toast(`Folder renamed to "${newName}".`, 'success');
      }

      // If we're currently viewing inside the renamed folder, update navigation
      if (state.poolPath === ctx.path || state.poolPath.startsWith(ctx.path + '/')) {
        state.poolPath = state.poolPath.replace(ctx.path, newPath);
      }

      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
      updateDropZoneTarget();
    } catch (e) {
      toast(`Rename folder failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

function openDeleteFolderDialog(folderPath, folderName) {
  state._deleteFolderCtx = { path: folderPath, name: folderName };
  const dlg = document.getElementById('delete-folder-dialog');
  const msg = document.getElementById('delete-folder-msg');
  const warning = document.getElementById('delete-folder-kit-warning');
  const kitMsg = document.getElementById('delete-folder-kit-msg');

  msg.textContent = `Delete folder "${folderName}" and all its contents?`;

  // Count files to be deleted
  const fileCount = state.files.filter(f =>
    f.path === folderPath || f.path.startsWith(folderPath + '/')
  ).length;

  // Check kit references
  const refs = findKitReferencesInFolder(folderPath);
  if (refs.length > 0) {
    warning.style.display = '';
    kitMsg.textContent = `${refs.length} sample(s) in the active kit reference files in this folder. Deleting will break those kit assignments and leave empty slots.`;
  } else {
    warning.style.display = 'none';
  }

  if (fileCount > 0) {
    msg.textContent = `Delete folder "${folderName}" containing ${fileCount} file(s)?`;
  }

  dlg.show();
}

function setupDeleteFolderDialog() {
  const dlg = document.getElementById('delete-folder-dialog');
  const ok  = document.getElementById('delete-folder-ok');
  const can = document.getElementById('delete-folder-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._deleteFolderCtx;
    if (!ctx) return;

    try {
      await deleteFolderOnDevice(ctx.path);

      // Clear out any kit entries that referenced this folder
      let cleared = 0;
      if (state.kitEntries) {
        for (let i = 0; i < state.kitEntries.length; i++) {
          const e = state.kitEntries[i];
          if (!e || !e.path) continue;
          if (e.path === ctx.path || e.path.startsWith(ctx.path + '/')) {
            state.kitEntries[i] = null;
            cleared++;
          }
        }
      }
      if (cleared > 0) {
        markDirty();
        await saveKit();
        toast(`Deleted folder and cleared ${cleared} kit reference(s).`, 'success');
      } else {
        toast(`Folder "${ctx.name}" deleted.`, 'success');
      }

      // If we're inside the deleted folder, navigate up
      if (state.poolPath === ctx.path || state.poolPath.startsWith(ctx.path + '/')) {
        // Go to parent folder
        const parts = ctx.path.split('/');
        parts.pop();
        state.poolPath = parts.join('/');
      }

      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
      updateCapacityBar();
      updateStorageBar();
      updateDropZoneTarget();
    } catch (e) {
      toast(`Delete folder failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

function openRenameDisplayName(idx, entry) {
  state._renameCtx = { _kitIndex: idx };
  const dlg = document.getElementById('rename-dialog');
  const inp = document.getElementById('rename-input');
  dlg.label = 'Edit Display Name';
  inp.value = entry.sname || entry.filename;
  dlg.show();
  setTimeout(() => inp.focus(), 100);

  const origOk = document.getElementById('rename-ok');
  const handler = async () => {
    const newName = document.getElementById('rename-input').value.trim();
    if (state.kitEntries[idx]) {
      state.kitEntries[idx].sname = newName;
      markDirty();
      renderKitEditor();
    }
    dlg.label = 'Rename';
    dlg.hide();
    origOk.removeEventListener('click', handler);
  };
  origOk.addEventListener('click', handler, { once: true });
}

function setupNewKitDialog() {
  const dlg   = document.getElementById('new-kit-dialog');
  const ok    = document.getElementById('new-kit-ok');
  const can   = document.getElementById('new-kit-cancel');
  const btn   = document.getElementById('new-kit-btn');
  const cloneCheckbox = document.getElementById('new-kit-clone');
  const optionsDiv = document.getElementById('new-kit-options');
  const bankCountInput = document.getElementById('new-kit-bank-count');
  const bankCountGroup = document.getElementById('new-kit-bank-count-group');

  // Bank count button group
  if (bankCountGroup) {
    bankCountGroup.addEventListener('click', e => {
      const clicked = e.target.closest('[data-bank-count]');
      if (!clicked) return;
      // Deselect all, select clicked
      bankCountGroup.querySelectorAll('[data-bank-count]').forEach(b => b.variant = 'default');
      clicked.variant = 'primary';
      bankCountInput.value = clicked.dataset.bankCount;
    });
  }

  // Hide bank options when cloning
  cloneCheckbox.addEventListener('sl-change', () => {
    optionsDiv.style.display = cloneCheckbox.checked ? 'none' : 'flex';
  });

  btn.addEventListener('click', () => {
    document.getElementById('new-kit-input').value = '';
    cloneCheckbox.checked = false;
    optionsDiv.style.display = 'flex';
    bankCountInput.value = '8';
    // Reset button group visual state
    if (bankCountGroup) {
      bankCountGroup.querySelectorAll('[data-bank-count]').forEach(b => {
        b.variant = b.dataset.bankCount === '8' ? 'primary' : 'default';
      });
    }
    document.getElementById('new-kit-naming').value = 'default';
    dlg.show();
  });

  ok.addEventListener('click', async () => {
    const name = document.getElementById('new-kit-input').value.trim();
    if (!name) { toast('Please enter a Kit name', 'warning'); return; }
    const clone = cloneCheckbox.checked;
    try {
      const entries = clone ? getKitForSave() : [];
      const result = await createKitOnDevice(name, entries);
      toast(`Kit "${name}" created`, 'success');
      const newIdx = result.newKitIndex !== undefined
        ? result.newKitIndex
        : (state.kits.smp_bank_names.length);
      state.kits.active_smp_bank = newIdx;
      state.dirty = false;
      await fetchSampleList();

      // If not cloning, apply bank count + naming options
      if (!clone) {
        const bankCount = parseInt(document.getElementById('new-kit-bank-count').value, 10) || 0;
        const naming = document.getElementById('new-kit-naming').value;
        const newBanks = [];
        for (let i = 0; i < bankCount; i++) {
          let bname;
          if (naming === 'default' && DEFAULT_BANKS[i]) {
            bname = DEFAULT_BANKS[i].name;
          } else if (naming === 'numbered' || (naming === 'default' && !DEFAULT_BANKS[i])) {
            bname = `BANK ${i + 1}`;
          } else {
            bname = '';
          }
          newBanks.push({
            name: bname,
            color: BANK_COLORS[i % BANK_COLORS.length],
            collapsed: false,
          });
        }
        state.banks = newBanks;
        // Ensure kitEntries covers all bank slots
        while (state.kitEntries.length < bankCount * SLICES_PER_BANK) {
          state.kitEntries.push(null);
        }
        // Trim kitEntries if fewer banks
        if (bankCount * SLICES_PER_BANK < state.kitEntries.length) {
          state.kitEntries.length = bankCount * SLICES_PER_BANK;
        }
      }

      renderKitSelector();
      renderKitEditor();
      updateCapacityBar();
      updateSaveButton();
    } catch (e) {
      toast(`Create Kit failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  DELETE KIT
// ═══════════════════════════════════════════════════════════════

function openDeleteKitDialog() {
  const dlg = document.getElementById('delete-kit-dialog');
  const inp = document.getElementById('delete-kit-confirm-input');
  const names = state.kits.smp_bank_names || [];
  const kitName = names[state.kits.active_smp_bank] || 'Default';
  document.getElementById('delete-kit-name').textContent = kitName;
  inp.value = '';
  document.getElementById('delete-kit-ok').setAttribute('disabled', '');
  dlg.show();
  setTimeout(() => inp.focus(), 100);
}

function setupDeleteKitDialog() {
  const dlg = document.getElementById('delete-kit-dialog');
  const ok  = document.getElementById('delete-kit-ok');
  const can = document.getElementById('delete-kit-cancel');
  const inp = document.getElementById('delete-kit-confirm-input');

  // Enable button only when user types the exact kit name
  inp.addEventListener('sl-input', () => {
    const names = state.kits.smp_bank_names || [];
    const kitName = names[state.kits.active_smp_bank] || 'Default';
    if (inp.value.trim() === kitName) {
      ok.removeAttribute('disabled');
    } else {
      ok.setAttribute('disabled', '');
    }
  });

  ok.addEventListener('click', async () => {
    const kitIdx = state.kits.active_smp_bank;
    try {
      await deleteKitOnDevice(kitIdx);
      toast('Kit deleted', 'success');
      state.kits.active_smp_bank = 0;
      state.dirty = false;
      await fetchSampleList();
      renderKitSelector();
      renderKitEditor();
      updateCapacityBar();
      updateSaveButton();
    } catch (e) {
      toast(`Delete Kit failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// --- Sample Picker ---
function openSamplePicker(bankIdx) {
  state._pickerBank = bankIdx;
  state._pickerSelected = new Set();
  renderPickerList('');
  document.getElementById('picker-search').value = '';
  document.getElementById('picker-dialog').show();
}

function renderPickerList(filter) {
  const list = document.getElementById('picker-list');
  const lc = filter.toLowerCase();
  const filtered = state.files.filter(f => {
    const stem = f.name.replace(/\.wav$/i, '');
    return !lc || stem.toLowerCase().includes(lc) || f.path.toLowerCase().includes(lc);
  });
  if (filtered.length === 0) {
    list.innerHTML = '<div style="padding:1rem;text-align:center;color:var(--sl-color-neutral-400);">No matching files.</div>';
    return;
  }
  list.innerHTML = filtered.map(f => {
    const stem = f.name.replace(/\.wav$/i, '');
    const key = `${f.path}/${stem}`;
    const sel = state._pickerSelected.has(key) ? ' selected' : '';
    return `<div class="sample-picker-item${sel}" data-key="${esc(key)}" data-path="${esc(f.path)}" data-name="${esc(stem)}" data-size="${f.size}">
      <sl-icon name="file-earmark-music" style="font-size:0.9rem;"></sl-icon>
      <span style="flex:1;">${esc(stem)}</span>
      <span style="font-size:0.72rem;color:var(--sl-color-neutral-500);">${esc(f.path)}</span>
      <span style="font-size:0.72rem;color:var(--sl-color-neutral-500);">${formatBytes(f.size)}</span>
    </div>`;
  }).join('');
}

function setupSamplePicker() {
  const dlg = document.getElementById('picker-dialog');
  const ok  = document.getElementById('picker-ok');
  const can = document.getElementById('picker-cancel');
  const search = document.getElementById('picker-search');
  const list = document.getElementById('picker-list');

  search.addEventListener('sl-input', () => renderPickerList(search.value));

  list.addEventListener('click', e => {
    const item = e.target.closest('.sample-picker-item');
    if (!item) return;
    const key = item.dataset.key;
    if (state._pickerSelected.has(key)) {
      state._pickerSelected.delete(key);
      item.classList.remove('selected');
    } else {
      state._pickerSelected.add(key);
      item.classList.add('selected');
    }
  });

  ok.addEventListener('click', () => {
    const bankIdx = state._pickerBank;
    let added = 0;
    for (const key of state._pickerSelected) {
      const [path, name] = [key.substring(0, key.lastIndexOf('/')), key.substring(key.lastIndexOf('/') + 1)];
      const file = state.files.find(f => f.path === path && f.name === name);
      const nsamp = file ? nsamples(file.size) : 0;
      if (addEntryToBank(bankIdx, name, path, nsamp)) added++;
    }
    if (added > 0) {
      markDirty();
      renderKitEditor();
      toast(`Added ${added} sample(s) to ${state.banks[bankIdx].name}`, 'success');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  SHOELACE DIALOGS — Bank & Kit confirmations
// ═══════════════════════════════════════════════════════════════

function openAddBankDialog() {
  const dlg = document.getElementById('add-bank-dialog');
  const inp = document.getElementById('add-bank-input');
  const ok  = document.getElementById('add-bank-ok');
  const can = document.getElementById('add-bank-cancel');
  inp.value = getNextUniqueBankName();
  dlg.show();
  setTimeout(() => inp.focus(), 100);

  const onOk = () => {
    const name = inp.value.trim();
    if (name) addBank(name);
    dlg.hide();
    cleanup();
  };
  const onCancel = () => { dlg.hide(); cleanup(); };
  const cleanup = () => {
    ok.removeEventListener('click', onOk);
    can.removeEventListener('click', onCancel);
  };
  ok.addEventListener('click', onOk);
  can.addEventListener('click', onCancel);
}

function openDeleteBankDialog(bankIdx, name, entryCount) {
  const dlg = document.getElementById('delete-bank-dialog');
  const msg = document.getElementById('delete-bank-msg');
  const ok  = document.getElementById('delete-bank-ok');
  const can = document.getElementById('delete-bank-cancel');
  msg.textContent = entryCount > 0
    ? `Delete Bank "${name}"? It contains ${entryCount} sample(s).`
    : `Delete Bank "${name}"?`;
  dlg.show();

  const onOk = () => {
    performDeleteBank(bankIdx, name);
    dlg.hide();
    cleanup();
  };
  const onCancel = () => { dlg.hide(); cleanup(); };
  const cleanup = () => {
    ok.removeEventListener('click', onOk);
    can.removeEventListener('click', onCancel);
  };
  ok.addEventListener('click', onOk);
  can.addEventListener('click', onCancel);
}

function openUnsavedKitDialog() {
  return new Promise(resolve => {
    const dlg = document.getElementById('unsaved-kit-dialog');
    const ok  = document.getElementById('unsaved-kit-ok');
    const can = document.getElementById('unsaved-kit-cancel');
    dlg.show();

    const onOk = () => { dlg.hide(); cleanup(); resolve(true); };
    const onCancel = () => { dlg.hide(); cleanup(); resolve(false); };
    const cleanup = () => {
      ok.removeEventListener('click', onOk);
      can.removeEventListener('click', onCancel);
    };
    ok.addEventListener('click', onOk);
    can.addEventListener('click', onCancel);
  });
}

// ═══════════════════════════════════════════════════════════════
//  TOOLBAR ACTIONS
// ═══════════════════════════════════════════════════════════════

function setupToolbar() {
  // Kit selector
  document.getElementById('kit-select').addEventListener('sl-change', async e => {
    if (state.initializing) return;
    if (state.dirty) {
      const proceed = await openUnsavedKitDialog();
      if (!proceed) {
        const sel = e.target;
        customElements.whenDefined('sl-select').then(() => {
          requestAnimationFrame(() => { sel.value = String(state.kits.active_smp_bank); });
        });
        return;
      }
    }
    const idx = parseInt(e.target.value, 10);
    state.kits.active_smp_bank = idx;
    state.dirty = false;
    try {
      await fetchSampleList();
      renderKitEditor();
      updateCapacityBar();
      updateSaveButton();
    } catch (err) {
      toast(`Failed to switch Kit: ${err.message}`, 'danger');
    }
  });

  // Save Kit
  document.getElementById('save-kit-btn').addEventListener('click', () => saveKit());

  // Delete Kit
  document.getElementById('delete-kit-btn').addEventListener('click', () => {
    const names = state.kits.smp_bank_names || [];
    if (names.length <= 1) {
      toast('Cannot delete the last kit', 'warning');
      return;
    }
    openDeleteKitDialog();
  });

  // View toggle
  document.getElementById('view-toggle').addEventListener('sl-change', e => {
    if (state.initializing) return;
    state.viewMode = e.target.checked ? 'banked' : 'flat';
    renderKitEditor();
  });

  // Export / Import Kit
  document.getElementById('export-kit-btn').addEventListener('click', () => exportKit());
  const importBtn = document.getElementById('import-kit-btn');
  const importInput = document.getElementById('import-kit-input');
  importBtn.addEventListener('click', () => importInput.click());
  importInput.addEventListener('change', () => {
    if (importInput.files.length > 0) {
      importKit(importInput.files[0]);
      importInput.value = '';
    }
  });

  // Theme toggle — delegated to app shell (shared.js) when running in unified mode
  if (!_S) setupThemeToggle();
}

// ═══════════════════════════════════════════════════════════════
//  KIT EXPORT / IMPORT (Backup & Restore)
// ═══════════════════════════════════════════════════════════════

function exportKit() {
  const names = state.kits.smp_bank_names || [];
  const kitName = names[state.kits.active_smp_bank] || 'Default';
  const entries = getKitForSave();
  const banksMeta = getBanksMeta();
  const exportData = {
    _format: 'tbd16-kit-backup',
    _version: 1,
    kitName,
    kitIndex: state.kits.active_smp_bank,
    exportDate: new Date().toISOString(),
    banks: banksMeta,
    entries: entries,
  };
  const json = JSON.stringify(exportData, null, 2);
  const blob = new Blob([json], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `${kitName.replace(/[^A-Za-z0-9_-]/g, '_')}_kit_backup.json`;
  a.click();
  URL.revokeObjectURL(url);
  toast(`Exported "${kitName}" kit backup`, 'success');
}

async function importKit(file) {
  try {
    const text = await file.text();
    const data = JSON.parse(text);
    if (data._format !== 'tbd16-kit-backup' || !data.entries) {
      toast('Invalid kit backup file', 'danger');
      return;
    }
    openImportKitDialog(data);
  } catch (e) {
    toast(`Import failed: ${e.message}`, 'danger');
  }
}

function openImportKitDialog(data) {
  const dlg = document.getElementById('import-kit-dialog');
  const body = document.getElementById('import-kit-body');

  // Build a lookup of available files for auto-mapping
  const available = new Map();
  for (const f of state.files) {
    const stem = f.name.replace(/\.wav$/i, '');
    // Index by filename for fuzzy matching
    if (!available.has(stem)) available.set(stem, []);
    available.get(stem).push(f);
  }

  // Analyze entries
  let totalSlots = 0, matched = 0, missing = 0;
  const entryAnalysis = [];
  for (let i = 0; i < data.entries.length; i++) {
    const e = data.entries[i];
    if (!e) continue;
    totalSlots++;
    const stem = (e.filename || '').replace(/\.wav$/i, '');
    const candidates = available.get(stem) || [];
    // Prefer exact path match, then any match
    const exact = candidates.find(f => f.path === e.path);
    const any = candidates.length > 0 ? candidates[0] : null;
    const found = exact || any;
    if (found) {
      matched++;
      entryAnalysis.push({ idx: i, entry: e, mapped: found, status: exact ? 'exact' : 'remapped' });
    } else {
      missing++;
      entryAnalysis.push({ idx: i, entry: e, mapped: null, status: 'missing' });
    }
  }

  // Build summary
  const bankNames = (data.banks || []).map(b => b.name).join(', ') || 'Unknown';
  const kitDate = data.exportDate ? new Date(data.exportDate).toLocaleDateString() : 'Unknown';

  let html = `<div style="margin-bottom:0.8rem;">
    <div style="font-weight:600;font-size:0.95rem;margin-bottom:0.3rem;">${esc(data.kitName)}</div>
    <div style="font-size:0.78rem;color:var(--sl-color-neutral-500);">Exported: ${esc(kitDate)} &middot; Banks: ${esc(bankNames)}</div>
  </div>
  <div style="display:flex;gap:1rem;margin-bottom:0.8rem;">
    <div style="flex:1;text-align:center;padding:0.5rem;border-radius:6px;background:var(--sl-color-success-100);color:var(--sl-color-success-700);font-size:0.82rem;">
      <strong>${matched}</strong> matched
    </div>
    <div style="flex:1;text-align:center;padding:0.5rem;border-radius:6px;background:${missing > 0 ? 'var(--sl-color-warning-100)' : 'var(--sl-color-neutral-100)'};color:${missing > 0 ? 'var(--sl-color-warning-700)' : 'var(--sl-color-neutral-500)'};font-size:0.82rem;">
      <strong>${missing}</strong> missing
    </div>
    <div style="flex:1;text-align:center;padding:0.5rem;border-radius:6px;background:var(--sl-color-neutral-100);color:var(--sl-color-neutral-600);font-size:0.82rem;">
      <strong>${totalSlots}</strong> total
    </div>
  </div>`;

  if (missing > 0) {
    html += `<div style="font-size:0.78rem;color:var(--sl-color-warning-600);margin-bottom:0.6rem;">
      <sl-icon name="exclamation-triangle" style="vertical-align:-2px;"></sl-icon>
      ${missing} sample(s) not found on SD card. Upload the missing files and re-import, or continue with partial restore.
    </div>`;
  }

  html += `<div style="max-height:240px;overflow-y:auto;border:1px solid var(--sl-color-neutral-200);border-radius:6px;margin-bottom:0.5rem;">
    <table style="width:100%;font-size:0.76rem;border-collapse:collapse;">
      <thead><tr style="background:var(--sl-color-neutral-100);position:sticky;top:0;">
        <th style="padding:0.3rem 0.5rem;text-align:left;">SAMPLE</th>
        <th style="padding:0.3rem 0.5rem;text-align:left;">BANK</th>
        <th style="padding:0.3rem 0.5rem;text-align:left;">STATUS</th>
      </tr></thead><tbody>`;

  for (const a of entryAnalysis) {
    const bankIdx = Math.floor(a.idx / SLICES_PER_BANK);
    const bankName = data.banks && data.banks[bankIdx] ? data.banks[bankIdx].name : `Bank ${bankIdx + 1}`;
    const statusIcon = a.status === 'exact'
      ? '<span style="color:var(--sl-color-success-600);">&#10003; Found</span>'
      : a.status === 'remapped'
      ? '<span style="color:var(--sl-color-primary-600);">&#8634; Remapped</span>'
      : '<span style="color:var(--sl-color-warning-600);">&#10007; Missing</span>';
    const rowBg = a.status === 'missing' ? 'background:color-mix(in srgb, var(--sl-color-warning-500) 8%, transparent);' : '';
    html += `<tr style="${rowBg}">
      <td style="padding:0.25rem 0.5rem;" title="${esc(a.entry.path + '/' + a.entry.filename)}">${esc(a.entry.filename)}</td>
      <td style="padding:0.25rem 0.5rem;">${esc(bankName)}</td>
      <td style="padding:0.25rem 0.5rem;">${statusIcon}</td>
    </tr>`;
  }

  html += '</tbody></table></div>';

  body.innerHTML = html;

  // Store analysis for the OK handler
  state._importData = { data, entryAnalysis };
  dlg.show();
}

function setupImportKitDialog() {
  const dlg = document.getElementById('import-kit-dialog');
  const ok  = document.getElementById('import-kit-ok');
  const can = document.getElementById('import-kit-cancel');

  ok.addEventListener('click', async () => {
    const { data, entryAnalysis } = state._importData || {};
    if (!data) { dlg.hide(); return; }

    // Apply banks metadata
    if (data.banks && data.banks.length > 0) {
      state.banks = data.banks.map((b, i) => ({
        name: b.name || `BANK ${i + 1}`,
        color: b.color || BANK_COLORS[i % BANK_COLORS.length],
        collapsed: false,
      }));
    }

    // Apply entries with auto-mapping
    const maxSlots = state.banks.length * SLICES_PER_BANK;
    state.kitEntries = [];
    for (let i = 0; i < maxSlots; i++) state.kitEntries.push(null);

    for (const a of entryAnalysis) {
      if (a.idx >= maxSlots) continue;
      if (a.mapped) {
        const stem = a.mapped.name.replace(/\.wav$/i, '');
        state.kitEntries[a.idx] = {
          filename: stem,
          path: a.mapped.path,
          nsamples: nsamples(a.mapped.size),
          sname: a.entry.sname || '',
        };
      } else {
        // Keep original entry marked as missing
        state.kitEntries[a.idx] = { ...a.entry };
      }
    }

    markMissingKitEntries();
    markDirty();
    renderKitEditor();

    const matchCount = entryAnalysis.filter(a => a.mapped).length;
    const missCount = entryAnalysis.filter(a => !a.mapped).length;
    toast(`Imported "${data.kitName}" — ${matchCount} mapped${missCount > 0 ? `, ${missCount} missing` : ''}`, 'success');
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  THEME TOGGLE — standalone fallback (used only on samples.html)
// ═══════════════════════════════════════════════════════════════

// sun-fill SVG for standalone mode
const _SM_SUN_SVG = '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16"><path d="M8 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8M8 0a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 0m0 13a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 13m8-5a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2a.5.5 0 0 1 .5.5M3 8a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2A.5.5 0 0 1 3 8m10.657-5.657a.5.5 0 0 1 0 .707l-1.414 1.415a.5.5 0 1 1-.707-.708l1.414-1.414a.5.5 0 0 1 .707 0m-9.193 9.193a.5.5 0 0 1 0 .707L3.05 13.657a.5.5 0 0 1-.707-.707l1.414-1.414a.5.5 0 0 1 .707 0m9.193 2.121a.5.5 0 0 1-.707 0l-1.414-1.414a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707M3.757 4.464a.5.5 0 0 1-.707 0L1.636 3.05a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707"/></svg>';

function setupThemeToggle() {
  const btn = document.getElementById('theme-toggle');
  if (!btn) return;
  const saved = localStorage.getItem('tbd-theme');
  if (saved === 'light') applyTheme('light');
  btn.addEventListener('click', () => {
    const isDark = document.documentElement.classList.contains('sl-theme-dark');
    applyTheme(isDark ? 'light' : 'dark');
  });
}

function applyTheme(theme) {
  const html = document.documentElement;
  const btn  = document.getElementById('theme-toggle');
  if (theme === 'light') {
    html.classList.remove('sl-theme-dark');
    html.classList.add('sl-theme-light');
    // Use inline SVG data URI to avoid network fetch for sun-fill icon
    btn.name = '';
    btn.src = `data:image/svg+xml,${encodeURIComponent(_SM_SUN_SVG)}`;
  } else {
    html.classList.remove('sl-theme-light');
    html.classList.add('sl-theme-dark');
    btn.src = '';
    btn.name = 'moon-fill';
  }
  const link = document.querySelector('link[href*="/shoelace/themes/"]');
  if (link) link.href = `/shoelace/themes/${theme}.css?v=2`;
  localStorage.setItem('tbd-theme', theme);
}

// ═══════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════

async function init() {
  // In unified mode, status is managed by app shell; standalone sets it directly
  const status = _S ? null : document.getElementById('status-text');

  // Setup all event handlers
  setupDropZone();
  setupPoolActions();
  setupKitActions();
  setupPoolDragEvents();
  setupToolbar();
  setupRenameDialog();
  setupDeleteDialog();
  setupNewFolderDialog();
  setupRenameFolderDialog();
  setupDeleteFolderDialog();
  setupNewKitDialog();
  setupDeleteKitDialog();
  setupSamplePicker();
  setupTransferBar();
  setupColumnSort();
  setupSelectionToolbar();
  setupBatchDeleteDialog();
  setupImportKitDialog();

  // Select mode button
  const selectBtn = document.getElementById('select-mode-btn');
  if (selectBtn) {
    selectBtn.addEventListener('click', () => toggleSelectionMode());
  }

  // Fetch initial data
  try {
    await fetchSampleList();
    if (status) {
      status.textContent = 'Connected';
      status.style.color = 'var(--sl-color-success-600)';
    }

    // Ensure user folder exists and navigate to it by default
    if (!state.folders.includes(USER_FOLDER)) {
      try { await createFolderOnDevice(USER_FOLDER); } catch {}
      await fetchSampleList();
    }
    state.poolPath = USER_FOLDER;
  } catch (e) {
    console.error('Initial fetch failed:', e);
    if (status) {
      status.textContent = 'Offline';
      status.style.color = 'var(--sl-color-danger-600)';
    }

    document.getElementById('pool-content').innerHTML =
      '<div style="color:var(--sl-color-neutral-400);font-size:0.8rem;padding:1rem;text-align:center;">Could not connect to device.</div>';
    document.getElementById('bank-container').innerHTML =
      '<div style="color:var(--sl-color-neutral-400);font-size:0.8rem;padding:1rem;text-align:center;">Connect to TBD-16 to manage samples.</div>';
    return;
  }

  // Render everything
  renderPoolContent();
  renderKitEditor();
  updateCapacityBar();
  updateStorageBar();
  updateDropZoneTarget();

  state.initializing = false;
}

// ═══════════════════════════════════════════════════════════════
//  EXPORT / AUTO-INIT
// ═══════════════════════════════════════════════════════════════

// Unified mode: export on window.TBD for lazy init from app.js
// Standalone mode (samples.html): auto-init on DOMContentLoaded
if (typeof window.TBD !== 'undefined' && window.TBD.shared) {
  window.TBD.sampleManager = { init: init, state: state };
} else {
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => setTimeout(init, 200));
  } else {
    setTimeout(init, 200);
  }
}

// ── app.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — App Shell
// Vanilla JS · Shoelace Web Components
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── Configuration Dialog ────────────────────────────────
  var currentConfig = null;

  async function loadConfiguration() {
    try {
      currentConfig = await S.queuedFetch('/device?action=getConfig');
      populateConfigDialog(currentConfig);
    } catch (e) {
      console.error('Failed to load config:', e);
    }
  }

  function populateConfigDialog(config) {
    // Connection tab
    var apiUrl = document.getElementById('cfg-api-url');
    if (apiUrl) apiUrl.value = config.apiEndpoint || window.location.origin;

    // MIDI tab
    var midiEnable = document.getElementById('cfg-midi-enable');
    if (midiEnable) midiEnable.checked = !!config.midiEnabled;

    var midiChannel = document.getElementById('cfg-midi-channel');
    if (midiChannel) {
      if (midiChannel.querySelectorAll('sl-option').length === 0) {
        var html = '';
        for (var i = 1; i <= 16; i++) {
          html += '<sl-option value="' + i + '">Channel ' + i + '</sl-option>';
        }
        midiChannel.innerHTML = html;
      }
      midiChannel.value = String(config.midiChannel || 1);
    }

    // Enumerate MIDI devices if Web MIDI is available
    populateMidiDevices();

    // WiFi tab — firmware stores in nested config.wifi object
    var wifi = config.wifi || {};
    var wifiSsid = document.getElementById('cfg-wifi-ssid');
    if (wifiSsid) wifiSsid.value = wifi.ssid || '';

    var wifiPassword = document.getElementById('cfg-wifi-password');
    if (wifiPassword) wifiPassword.value = wifi.pwd || '';

    var wifiMdns = document.getElementById('cfg-wifi-mdns');
    if (wifiMdns) wifiMdns.value = wifi.mdns_name || '';

    var wifiMode = wifi.mode || 'ap';
    var wifiAp = document.getElementById('cfg-wifi-ap');
    var wifiSta = document.getElementById('cfg-wifi-sta');
    var wifiUsbncm = document.getElementById('cfg-wifi-usbncm');
    if (wifiAp) wifiAp.checked = (wifiMode === 'ap');
    if (wifiSta) wifiSta.checked = (wifiMode === 'sta');
    if (wifiUsbncm) wifiUsbncm.checked = (wifiMode === 'usbncm');

    // Audio tab — firmware uses ch0_codecLvlOut (0-63) and ch0_outputSoftClip ("on"/"off")
    var ch0Level = document.getElementById('cfg-input-gain');
    var ch0LevelVal = document.getElementById('cfg-input-gain-val');
    if (ch0Level) {
      var lvl0 = parseInt(config.ch0_codecLvlOut) || 58;
      ch0Level.min = 0; ch0Level.max = 63;
      ch0Level.value = lvl0;
      if (ch0LevelVal) ch0LevelVal.textContent = lvl0 + ' / 63';
    }
    var ch1Level = document.getElementById('cfg-output-gain');
    var ch1LevelVal = document.getElementById('cfg-output-gain-val');
    if (ch1Level) {
      var lvl1 = parseInt(config.ch1_codecLvlOut) || 58;
      ch1Level.min = 0; ch1Level.max = 63;
      ch1Level.value = lvl1;
      if (ch1LevelVal) ch1LevelVal.textContent = lvl1 + ' / 63';
    }
    var noiseGate = document.getElementById('cfg-noise-gate');
    if (noiseGate) noiseGate.checked = !!config.noiseGate;
    var softClipCh0 = document.getElementById('cfg-soft-clip-ch0');
    if (softClipCh0) softClipCh0.checked = config.ch0_outputSoftClip === 'on';
    var softClipCh1 = document.getElementById('cfg-soft-clip-ch1');
    if (softClipCh1) softClipCh1.checked = config.ch1_outputSoftClip === 'on';

    // Channel routing
    var daisyChain = document.getElementById('cfg-daisy-chain');
    if (daisyChain) daisyChain.value = config.ch01_daisy || 'off';
    var ch0Stereo = document.getElementById('cfg-ch0-stereo');
    if (ch0Stereo) ch0Stereo.value = config.ch0_toStereo || 'off';
    var ch1Stereo = document.getElementById('cfg-ch1-stereo');
    if (ch1Stereo) ch1Stereo.value = config.ch1_toStereo || 'off';

    // Appearance tab
    var compact = document.getElementById('cfg-compact');
    if (compact) compact.checked = !!config.compactLayout;

    // System tab
    var firmware = document.getElementById('cfg-firmware');
    if (firmware) firmware.textContent = config.firmwareVersion || '—';

    var hardware = document.getElementById('cfg-hardware');
    if (hardware) hardware.textContent = config.hardwareVersion || config.codec || '—';

    var samplerate = document.getElementById('cfg-samplerate');
    if (samplerate) samplerate.textContent = config.sampleRate ? (config.sampleRate + ' Hz') : '—';

    var connstatus = document.getElementById('cfg-connstatus');
    if (connstatus) {
      var dot = connstatus.querySelector('.status-footer-dot');
      if (dot) dot.style.background = '#4caf50';
      connstatus.lastChild.textContent = ' Connected';
    }
  }

  function populateMidiDevices() {
    var container = document.getElementById('cfg-midi-devices');
    if (!container) return;

    if (navigator.requestMIDIAccess) {
      navigator.requestMIDIAccess().then(function(access) {
        var html = '';
        var inputs = access.inputs;
        var outputs = access.outputs;
        var deviceNames = {};

        inputs.forEach(function(input) {
          deviceNames[input.name] = deviceNames[input.name] || { input: false, output: false };
          deviceNames[input.name].input = true;
        });
        outputs.forEach(function(output) {
          deviceNames[output.name] = deviceNames[output.name] || { input: false, output: false };
          deviceNames[output.name].output = true;
        });

        var names = Object.keys(deviceNames);
        if (names.length === 0) {
          container.innerHTML = '<div style="font-size:0.82rem;color:var(--sl-color-neutral-500);padding:0.5rem 0;">No MIDI devices detected</div>';
          return;
        }

        names.forEach(function(name) {
          var d = deviceNames[name];
          var badges = '';
          if (d.input) badges += '<span class="midi-device-badge">Input</span>';
          if (d.output) badges += '<span class="midi-device-badge">Output</span>';
          html += '<div class="midi-device-item"><span style="font-size:0.85rem;">' + S.esc(name) + '</span><div>' + badges + '</div></div>';
        });
        container.innerHTML = html;
      }).catch(function() {
        container.innerHTML = '<div style="font-size:0.82rem;color:var(--sl-color-neutral-500);padding:0.5rem 0;">MIDI access denied</div>';
      });
    } else {
      container.innerHTML = '<div style="font-size:0.82rem;color:var(--sl-color-neutral-500);padding:0.5rem 0;">Web MIDI not supported in this browser</div>';
    }
  }

  function setupConfigDialog() {
    // Config tab switching
    var configTabs = document.getElementById('config-tabs');
    if (configTabs) {
      configTabs.addEventListener('click', function(e) {
        var tab = e.target.closest('.config-tab');
        if (!tab) return;
        var target = tab.getAttribute('data-tab');
        configTabs.querySelectorAll('.config-tab').forEach(function(t) { t.classList.remove('active'); });
        tab.classList.add('active');
        document.querySelectorAll('#config-content .config-tab-panel').forEach(function(p) { p.classList.remove('active'); });
        var panel = document.getElementById(target);
        if (panel) panel.classList.add('active');
      });
    }

    // Config button
    var configBtn = document.getElementById('config-btn');
    if (configBtn) {
      configBtn.addEventListener('click', function() {
        loadConfiguration();
        document.getElementById('config-dialog').show();
      });
    }

    // Save
    var configSave = document.getElementById('config-save');
    if (configSave) configSave.addEventListener('click', saveConfiguration);

    // Cancel
    var configCancel = document.getElementById('config-cancel');
    if (configCancel) {
      configCancel.addEventListener('click', function() {
        document.getElementById('config-dialog').hide();
      });
    }

    // Reboot button in system tab
    var cfgReboot = document.getElementById('cfg-reboot');
    if (cfgReboot) {
      cfgReboot.addEventListener('click', function() {
        document.getElementById('config-dialog').hide();
        document.getElementById('reboot-dialog').show();
      });
    }

    // Test Connection button
    var testConn = document.getElementById('cfg-test-connection');
    if (testConn) {
      testConn.addEventListener('click', async function() {
        var dot = document.getElementById('cfg-conn-dot');
        var statusEl = document.getElementById('cfg-conn-status');
        if (dot) dot.style.background = '#ff9800';
        if (statusEl) statusEl.textContent = 'Testing…';
        try {
          var apiUrl = document.getElementById('cfg-api-url');
          var url = (apiUrl ? apiUrl.value : window.location.origin) + '/api/v2/device?action=getIOCaps';
          var resp = await S.apiQueue.enqueue(function() {
            return fetch(url, { method: 'GET', signal: AbortSignal.timeout(5000) });
          });
          if (resp.ok) {
            if (dot) dot.style.background = '#4caf50';
            if (statusEl) statusEl.textContent = 'Connected';
          } else {
            if (dot) dot.style.background = '#f44336';
            if (statusEl) statusEl.textContent = 'Error (' + resp.status + ')';
          }
        } catch (e) {
          if (dot) dot.style.background = '#f44336';
          if (statusEl) statusEl.textContent = 'Unreachable';
        }
      });
    }

    // WiFi Save button
    var wifiSave = document.getElementById('cfg-wifi-save');
    if (wifiSave) {
      wifiSave.addEventListener('click', async function() {
        if (!currentConfig) {
          S.toast('Configuration not loaded yet', 'warning');
          return;
        }
        var mode = 'ap';
        if (document.getElementById('cfg-wifi-sta') && document.getElementById('cfg-wifi-sta').checked) mode = 'sta';
        if (document.getElementById('cfg-wifi-usbncm') && document.getElementById('cfg-wifi-usbncm').checked) mode = 'usbncm';
        var ssid = document.getElementById('cfg-wifi-ssid');
        var password = document.getElementById('cfg-wifi-password');
        var mdns = document.getElementById('cfg-wifi-mdns');
        // Validate password length (firmware requires >= 8 or empty)
        var pwd = password ? password.value : '';
        if (pwd.length > 0 && pwd.length < 8) {
          S.toast('Password must be at least 8 characters, or empty', 'warning');
          return;
        }
        // Merge WiFi into firmware's nested wifi object (config.wifi)
        var config = currentConfig;
        if (!config.wifi) config.wifi = {};
        config.wifi.mode = mode;
        config.wifi.ssid = ssid ? ssid.value : '';
        config.wifi.pwd = pwd;
        config.wifi.mdns_name = mdns ? mdns.value : '';
        try {
          await S.queuedPost('/device?action=setConfig', config);
          currentConfig = config;
          S.toast('WiFi settings saved. Reboot for changes to take effect.', 'warning', 5000);
        } catch (e) {
          S.toast('Failed to save WiFi settings', 'danger');
        }
      });
    }

    // Firmware Update button
    var fwUpdate = document.getElementById('cfg-firmware-update');
    if (fwUpdate) {
      fwUpdate.addEventListener('click', function() {
        S.toast('Firmware update not yet implemented', 'warning');
      });
    }

    // Factory Reset button
    var factoryReset = document.getElementById('cfg-factory-reset');
    if (factoryReset) {
      factoryReset.addEventListener('click', function() {
        if (confirm('Are you sure you want to factory reset? This will erase all presets and settings.')) {
          S.toast('Factory reset not yet implemented', 'warning');
        }
      });
    }

    // Backup / Restore buttons
    var backupBtn = document.getElementById('cfg-backup');
    if (backupBtn) {
      backupBtn.addEventListener('click', async function() {
        try {
          S.showLoading('Creating backup…');
          var backup = {};

          // 1) Configuration
          backup.configuration = await S.queuedFetch('/device?action=getConfig');

          // 2) Favorites
          backup.favorites = await S.queuedFetch('/device?action=getFavorites');

          // 3) All plugin preset data
          var plugins = await S.queuedFetch('/plugins?action=list');
          backup.presets = {};
          for (var i = 0; i < plugins.length; i++) {
            var pid = plugins[i].id;
            if (pid === 'Void') continue;
            try {
              backup.presets[pid] = await S.queuedFetch('/plugins?action=getPresetData&id=' + encodeURIComponent(pid));
            } catch (e) {
              // Plugin may not have preset data — skip silently
            }
          }

          // 4) Write as JSON download
          var data = JSON.stringify(backup, null, 2);
          var blob = new Blob([data], { type: 'application/json' });
          var url = URL.createObjectURL(blob);
          var a = document.createElement('a');
          a.href = url;
          var dateStr = new Date().toISOString().slice(0, 10);
          a.download = 'ctag-tbd-backup-' + dateStr + '.json';
          document.body.appendChild(a);
          a.click();
          document.body.removeChild(a);
          URL.revokeObjectURL(url);

          S.hideLoading();
          S.toast('Backup created', 'success', 2000);
        } catch (e) {
          S.hideLoading();
          S.toast('Backup failed: ' + e.message, 'danger');
        }
      });
    }
    var restoreBtn = document.getElementById('cfg-restore');
    if (restoreBtn) {
      restoreBtn.addEventListener('click', function() {
        var input = document.createElement('input');
        input.type = 'file';
        input.accept = '.json';
        input.addEventListener('change', async function() {
          if (!input.files || !input.files[0]) return;
          if (!confirm('This will overwrite ALL settings, presets, and favorites. Continue?')) return;
          try {
            S.showLoading('Restoring backup…');
            var text = await input.files[0].text();
            var backup = JSON.parse(text);

            // 1) Restore configuration
            if (backup.configuration) {
              await S.queuedPost('/device?action=setConfig', backup.configuration);
            }

            // 2) Restore favorites
            if (Array.isArray(backup.favorites)) {
              for (var i = 0; i < backup.favorites.length; i++) {
                if (backup.favorites[i] && backup.favorites[i].plug_0) {
                  await S.queuedPost('/device?action=storeFavorite&id=' + i, backup.favorites[i]);
                }
              }
            }

            // 3) Restore preset data
            if (backup.presets) {
              var pluginIds = Object.keys(backup.presets);
              for (var j = 0; j < pluginIds.length; j++) {
                var pid = pluginIds[j];
                try {
                  await S.queuedPost('/plugins?action=setPresetData&id=' + encodeURIComponent(pid), backup.presets[pid]);
                } catch (e) {
                  // Non-critical — plugin may not exist on this device
                }
              }
            }

            S.hideLoading();
            S.toast('Backup restored — please reload the page', 'success', 5000);
          } catch (e) {
            S.hideLoading();
            S.toast('Restore failed: ' + e.message, 'danger');
          }
        });
        input.click();
      });
    }

    // Audio tab - codec level range labels
    var ch0Level = document.getElementById('cfg-input-gain');
    var ch0LevelVal = document.getElementById('cfg-input-gain-val');
    if (ch0Level && ch0LevelVal) {
      ch0Level.addEventListener('input', function() {
        var v = parseInt(ch0Level.value);
        ch0LevelVal.textContent = v + ' / 63';
      });
    }
    var ch1Level = document.getElementById('cfg-output-gain');
    var ch1LevelVal = document.getElementById('cfg-output-gain-val');
    if (ch1Level && ch1LevelVal) {
      ch1Level.addEventListener('input', function() {
        var v = parseInt(ch1Level.value);
        ch1LevelVal.textContent = v + ' / 63';
      });
    }
    // Audio save button
    var audioSave = document.getElementById('cfg-audio-save');
    if (audioSave) {
      audioSave.addEventListener('click', async function() {
        if (!currentConfig) {
          S.toast('Configuration not loaded yet', 'warning');
          return;
        }
        // Merge audio settings using firmware's actual keys
        var config = currentConfig;
        config.ch0_codecLvlOut = ch0Level ? String(parseInt(ch0Level.value)) : '58';
        config.ch1_codecLvlOut = ch1Level ? String(parseInt(ch1Level.value)) : '58';
        var scCh0 = document.getElementById('cfg-soft-clip-ch0');
        var scCh1 = document.getElementById('cfg-soft-clip-ch1');
        config.ch0_outputSoftClip = (scCh0 && scCh0.checked) ? 'on' : 'off';
        config.ch1_outputSoftClip = (scCh1 && scCh1.checked) ? 'on' : 'off';
        // Channel routing
        var daisy = document.getElementById('cfg-daisy-chain');
        if (daisy) config.ch01_daisy = daisy.value;
        var ch0s = document.getElementById('cfg-ch0-stereo');
        if (ch0s) config.ch0_toStereo = ch0s.value;
        var ch1s = document.getElementById('cfg-ch1-stereo');
        if (ch1s) config.ch1_toStereo = ch1s.value;
        try {
          await S.queuedPost('/device?action=setConfig', config);
          currentConfig = config;
          S.toast('Audio settings saved', 'success');
        } catch (e) {
          S.toast('Failed to save audio settings', 'danger');
        }
      });
    }

    // Palette selection
    var paletteGrid = document.getElementById('palette-grid');
    if (paletteGrid) {
      paletteGrid.addEventListener('click', function(e) {
        var card = e.target.closest('.palette-card');
        if (!card) return;
        paletteGrid.querySelectorAll('.palette-card').forEach(function(c) { c.classList.remove('active'); });
        card.classList.add('active');
        var paletteName = card.getAttribute('data-palette');
        localStorage.setItem('tbd-palette', paletteName);
        applyPalette(paletteName);
      });
      // Restore saved palette
      var saved = localStorage.getItem('tbd-palette');
      if (saved) {
        var savedCard = paletteGrid.querySelector('[data-palette="' + saved + '"]');
        if (savedCard) {
          paletteGrid.querySelectorAll('.palette-card').forEach(function(c) { c.classList.remove('active'); });
          savedCard.classList.add('active');
          applyPalette(saved);
        }
      }
    }

    // Control Mode toggle
    var controlModeSwitch = document.getElementById('cfg-control-mode');
    if (controlModeSwitch) {
      // Restore saved state
      if (S.isControlMode()) {
        controlModeSwitch.checked = true;
        S.loadWebAudioControls().catch(function(e) {
          console.warn('webaudio-controls load failed:', e);
        });
      }
      controlModeSwitch.addEventListener('sl-change', function() {
        S.setControlMode(controlModeSwitch.checked);
        // Re-render params in both slots to reflect mode change
        if (window.TBD.pluginManager && window.TBD.pluginManager.rerenderParams) {
          window.TBD.pluginManager.rerenderParams();
        }
      });
    }
  }

  var palettes = {
    'rams-warm': {
      primary: '#c87533',
      neutral: '#5c5c4a',
    },
    'rams-contrast': {
      primary: '#d04a35',
      neutral: '#2d2d2d',
    },
    'rams-muted': {
      primary: '#c05a3c',
      neutral: '#3d3d2e',
    },
  };

  function applyPalette(name) {
    var p = palettes[name];
    if (!p) return;
    // Apply primary accent color as CSS custom property on :root
    document.documentElement.style.setProperty('--sl-color-primary-600', p.primary);
    // Lighter variant for hover
    document.documentElement.style.setProperty('--sl-color-primary-500', lightenColor(p.primary, 15));
    document.documentElement.style.setProperty('--sl-color-primary-700', darkenColor(p.primary, 15));
  }

  function lightenColor(hex, pct) {
    var r = parseInt(hex.slice(1,3), 16);
    var g = parseInt(hex.slice(3,5), 16);
    var b = parseInt(hex.slice(5,7), 16);
    r = Math.min(255, Math.round(r + (255 - r) * pct / 100));
    g = Math.min(255, Math.round(g + (255 - g) * pct / 100));
    b = Math.min(255, Math.round(b + (255 - b) * pct / 100));
    return '#' + [r, g, b].map(function(c) { return c.toString(16).padStart(2, '0'); }).join('');
  }

  function darkenColor(hex, pct) {
    var r = parseInt(hex.slice(1,3), 16);
    var g = parseInt(hex.slice(3,5), 16);
    var b = parseInt(hex.slice(5,7), 16);
    r = Math.max(0, Math.round(r * (1 - pct / 100)));
    g = Math.max(0, Math.round(g * (1 - pct / 100)));
    b = Math.max(0, Math.round(b * (1 - pct / 100)));
    return '#' + [r, g, b].map(function(c) { return c.toString(16).padStart(2, '0'); }).join('');
  }

  async function saveConfiguration() {
    if (!currentConfig) {
      S.toast('Configuration not loaded yet', 'warning');
      return;
    }
    var config = currentConfig;

    var apiUrl = document.getElementById('cfg-api-url');
    if (apiUrl) config.apiEndpoint = apiUrl.value;

    var midiEnable = document.getElementById('cfg-midi-enable');
    if (midiEnable) config.midiEnabled = midiEnable.checked;

    var midiChannel = document.getElementById('cfg-midi-channel');
    if (midiChannel) config.midiChannel = parseInt(midiChannel.value, 10) || 1;

    var compact = document.getElementById('cfg-compact');
    if (compact) config.compactLayout = compact.checked;

    try {
      await S.queuedPost('/device?action=setConfig', config);
      S.toast('Configuration saved', 'success');
      document.getElementById('config-dialog').hide();
    } catch (e) {
      S.toast('Failed to save configuration', 'danger');
    }
  }

  // ─── Debug Panel ──────────────────────────────────────────

  var debugOpen = false;

  function setupDebugPanel() {
    var toggle = document.getElementById('debug-toggle');
    var panel = document.getElementById('debug-panel');
    var closeBtn = document.getElementById('debug-close');
    var refreshBtn = document.getElementById('debug-refresh');

    if (toggle && panel) {
      toggle.addEventListener('click', function() {
        debugOpen = !debugOpen;
        panel.classList.toggle('expanded', debugOpen);
        if (debugOpen) refreshDebugPanel();
      });
    }

    if (closeBtn && panel) {
      closeBtn.addEventListener('click', function() {
        debugOpen = false;
        panel.classList.remove('expanded');
      });
    }

    if (refreshBtn) {
      refreshBtn.addEventListener('click', refreshDebugPanel);
    }

    // Debug tab switching
    var debugTabs = panel ? panel.querySelectorAll('.debug-tab') : [];
    debugTabs.forEach(function(tab) {
      tab.addEventListener('click', function() {
        debugTabs.forEach(function(t) { t.classList.remove('active'); });
        tab.classList.add('active');
        panel.querySelectorAll('.debug-tab-panel').forEach(function(p) { p.classList.remove('active'); });
        var target = document.getElementById(tab.getAttribute('data-tab'));
        if (target) target.classList.add('active');
      });
    });
  }

  // ─── API Call Tracking ─────────────────────────────────────
  var debugApiCalls = 0;
  var debugApiErrors = 0;
  var debugLastApi = '—';
  var debugLastError = 'None';
  var debugParamChanges = 0;
  var debugApiLog = [];
  var MAX_API_LOG = 50;

  // Wrap S.apiFetch to track API calls
  (function() {
    if (!S || !S.apiFetch) return;
    var origFetch = S.apiFetch;
    S.apiFetch = async function(url) {
      debugApiCalls++;
      var shortUrl = url.replace(/^\/api\/v2/, '');
      debugLastApi = shortUrl;
      var ts = new Date().toLocaleTimeString();
      debugApiLog.unshift(ts + ' ' + shortUrl);
      if (debugApiLog.length > MAX_API_LOG) debugApiLog.pop();
      try {
        var result = await origFetch.apply(this, arguments);
        return result;
      } catch (e) {
        debugApiErrors++;
        debugLastError = shortUrl + ' — ' + (e.message || String(e));
        throw e;
      }
    };
  })();

  // Track parameter changes from plugin-manager
  S.onParamChange = function() { debugParamChanges++; };

  function refreshDebugPanel() {
    var pm = window.TBD.pluginManager;
    var pmState = pm ? pm.state : {};
    var sm = window.TBD.sampleManager;

    function set(id, val) {
      var el = document.getElementById(id);
      if (el) el.textContent = val;
    }

    // App State
    set('dbg-active-view', activeView || '—');
    set('dbg-slot-a', (pmState.activePlugin && pmState.activePlugin[0]) ? pmState.activePlugin[0].id : 'None');
    set('dbg-slot-b', (pmState.activePlugin && pmState.activePlugin[1]) ? pmState.activePlugin[1].id : 'None');
    set('dbg-stereo', pmState.stereoLocked ? 'Yes' : 'No');
    set('dbg-plugin-count', pmState.plugins ? pmState.plugins.length : '0');

    // Preset info
    var presetA = '—', presetB = '—';
    if (pmState.presets && pmState.presets[0] && pmState.activePreset) {
      var pa = pmState.presets[0].find(function(p) { return p.number === pmState.activePreset[0]; });
      presetA = pa ? pa.name + ' (#' + pa.number + ')' : (pmState.activePreset[0] >= 0 ? '#' + pmState.activePreset[0] : '—');
    }
    if (pmState.presets && pmState.presets[1] && pmState.activePreset) {
      var pb = pmState.presets[1].find(function(p) { return p.number === pmState.activePreset[1]; });
      presetB = pb ? pb.name + ' (#' + pb.number + ')' : (pmState.activePreset[1] >= 0 ? '#' + pmState.activePreset[1] : '—');
    }
    set('dbg-preset-a', presetA);
    set('dbg-preset-b', presetB);

    // Favorites
    set('dbg-fav-count', pm && pm.favoritesCache ? Object.keys(pm.favoritesCache).length : '0');

    // Kit info
    if (sm && sm.state) {
      var kitNames = sm.state.kits ? sm.state.kits.smp_bank_names || [] : [];
      var activeKit = sm.state.kits ? sm.state.kits.active_smp_bank : 0;
      var kitName = kitNames[activeKit] || 'Kit ' + activeKit;
      var bankCount = sm.state.banks ? sm.state.banks.length : 0;
      var sampleCount = sm.state.kitEntries ? sm.state.kitEntries.filter(Boolean).length : 0;
      set('dbg-kit-info', kitName + ' — ' + bankCount + ' banks, ' + sampleCount + ' samples');
    } else {
      set('dbg-kit-info', 'Not loaded');
    }

    // Page load time
    if (window.performance && window.performance.timing) {
      var t = window.performance.timing;
      var loadMs = t.loadEventEnd - t.navigationStart;
      set('dbg-load-time', loadMs > 0 ? loadMs + ' ms' : '—');
    }

    // Network
    set('dbg-ws-status', S.connectionState ? (S.connectionState.connected ? '● Connected' : '○ Disconnected') : '—');
    set('dbg-api-url', S.API_BASE || window.location.origin);
    set('dbg-api-calls', String(debugApiCalls));
    set('dbg-api-errors', String(debugApiErrors));
    set('dbg-last-api', debugLastApi);
    set('dbg-last-error', debugLastError);

    // API log
    var logEl = document.getElementById('dbg-api-log');
    if (logEl) {
      logEl.textContent = debugApiLog.length > 0 ? debugApiLog.join('\n') : 'No API calls logged yet.';
    }

    // Parameters
    var paramCountA = 0, paramCountB = 0, groupCountA = 0, groupCountB = 0;
    if (pmState.params && pmState.params[0] && pmState.params[0].params) {
      paramCountA = pmState.params[0].params.length;
      groupCountA = pmState.params[0].params.filter(function(p) { return p.type === 'group'; }).length;
    }
    if (pmState.params && pmState.params[1] && pmState.params[1].params) {
      paramCountB = pmState.params[1].params.length;
      groupCountB = pmState.params[1].params.filter(function(p) { return p.type === 'group'; }).length;
    }
    set('dbg-params-a', String(paramCountA));
    set('dbg-groups-a', String(groupCountA));
    set('dbg-params-b', String(paramCountB));
    set('dbg-groups-b', String(groupCountB));
    set('dbg-param-changes', String(debugParamChanges));
  }

  // ─── View Switching ───────────────────────────────────────
  var activeView = 'view-plugins';
  var sampleManagerInited = false;

  function switchView(viewId) {
    if (viewId === activeView) return;

    // Toggle active class on view containers
    var views = document.querySelectorAll('#view-plugins, #view-samples');
    views.forEach(function(v) { v.classList.remove('active'); });
    var target = document.getElementById(viewId);
    if (target) target.classList.add('active');

    // Toggle active class on nav tabs
    var tabs = document.querySelectorAll('.nav-tab');
    tabs.forEach(function(t) { t.classList.remove('active'); });
    var tab = document.querySelector('.nav-tab[data-view="' + viewId + '"]');
    if (tab) tab.classList.add('active');

    // Show/hide storage bar (only visible in sample view)
    var storageBar = document.getElementById('header-storage');
    if (storageBar) {
      if (viewId === 'view-samples') {
        storageBar.classList.remove('hidden');
      } else {
        storageBar.classList.add('hidden');
      }
    }

    // Show/hide favorites bar (only visible in plugin view)
    var favBar = document.getElementById('favorites-bar');
    if (favBar) {
      if (viewId === 'view-samples') {
        favBar.classList.add('hidden');
      } else {
        favBar.classList.remove('hidden');
      }
    }

    activeView = viewId;

    // Lazy-init Sample Manager on first switch
    if (viewId === 'view-samples' && !sampleManagerInited) {
      sampleManagerInited = true;
      if (window.TBD.sampleManager) {
        window.TBD.sampleManager.init();
      }
    }

    // Update URL without reload
    var url = new URL(window.location);
    url.searchParams.set('view', viewId === 'view-samples' ? 'samples' : 'plugins');
    history.replaceState(null, '', url);
  }

  // ─── Setup ───────────────────────────────────────────────
  function setup() {
    // Theme toggle
    S.setupThemeToggle('theme-toggle');

    // Nav tab click handlers
    var navTabs = document.querySelectorAll('.nav-tab[data-view]');
    navTabs.forEach(function(tab) {
      tab.addEventListener('click', function() {
        switchView(tab.getAttribute('data-view'));
      });
    });

    // Config dialog (tabbed)
    setupConfigDialog();

    // Debug panel
    setupDebugPanel();

    // Reboot confirm
    var rebootOk = document.getElementById('reboot-ok');
    if (rebootOk) {
      rebootOk.addEventListener('click', async function() {
        document.getElementById('reboot-dialog').hide();
        try {
          await S.queuedPost('/device?action=reboot', null);
          S.toast('Rebooting…', 'warning', 6000);
          S.setDisconnected();
        } catch (e) {
          // Expected — device reboots immediately
          S.toast('Rebooting…', 'warning', 6000);
          S.setDisconnected();
        }
      });
    }
    var rebootCancel = document.getElementById('reboot-cancel');
    if (rebootCancel) {
      rebootCancel.addEventListener('click', function() {
        document.getElementById('reboot-dialog').hide();
      });
    }

    // Connection monitor — only used for reconnect detection
    S.startConnectionMonitor(
      async function onConnect() {
        // Skip the first connect (handled by init() below)
        if (!S.connectionState._firstConnectDone) return;
        // Refresh on reconnect — sequential to avoid socket exhaustion
        if (window.TBD.pluginManager) {
          await window.TBD.pluginManager.init();
        }
        // Refresh sample manager if it was initialized
        if (sampleManagerInited && window.TBD.sampleManager) {
          await window.TBD.sampleManager.init();
        }
      },
      function onDisconnect() {
        // Nothing extra needed — UI updates via shared.js
      }
    );
  }

  // ─── Init ────────────────────────────────────────────────
  function init() {
    setup();

    // Check URL for initial view
    var params = new URLSearchParams(window.location.search);
    var requestedView = params.get('view');

    // Initialize plugin manager (first load)
    if (window.TBD.pluginManager) {
      window.TBD.pluginManager.init().then(function() {
        // Only set connected if circuit breaker didn't trigger disconnect during init
        if (S.connectionState.status !== 'disconnected') {
          S.setConnected();
        }
        // Mark first connect done after a tick so onConnect callback doesn't re-trigger
        setTimeout(function() {
          S.connectionState._firstConnectDone = true;
        }, 100);
      }).catch(function() {
        S.setDisconnected();
      });
    }

    // If ?view=samples was requested, switch to it after a short delay
    if (requestedView === 'samples') {
      setTimeout(function() { switchView('view-samples'); }, 300);
    }
  }

  // Boot
  document.addEventListener('DOMContentLoaded', function() {
    setTimeout(init, 150);
  });

})();

