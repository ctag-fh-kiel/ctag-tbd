// ── Sortable.min.js ───
/*! Sortable 1.10.2 - MIT | git://github.com/SortableJS/Sortable.git */
!function(t,e){"object"==typeof exports&&"undefined"!=typeof module?module.exports=e():"function"==typeof define&&define.amd?define(e):(t=t||self).Sortable=e()}(this,function(){"use strict";function o(t){return(o="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function(t){return typeof t}:function(t){return t&&"function"==typeof Symbol&&t.constructor===Symbol&&t!==Symbol.prototype?"symbol":typeof t})(t)}function a(){return(a=Object.assign||function(t){for(var e=1;e<arguments.length;e++){var n=arguments[e];for(var o in n)Object.prototype.hasOwnProperty.call(n,o)&&(t[o]=n[o])}return t}).apply(this,arguments)}function I(i){for(var t=1;t<arguments.length;t++){var r=null!=arguments[t]?arguments[t]:{},e=Object.keys(r);"function"==typeof Object.getOwnPropertySymbols&&(e=e.concat(Object.getOwnPropertySymbols(r).filter(function(t){return Object.getOwnPropertyDescriptor(r,t).enumerable}))),e.forEach(function(t){var e,n,o;e=i,o=r[n=t],n in e?Object.defineProperty(e,n,{value:o,enumerable:!0,configurable:!0,writable:!0}):e[n]=o})}return i}function l(t,e){if(null==t)return{};var n,o,i=function(t,e){if(null==t)return{};var n,o,i={},r=Object.keys(t);for(o=0;o<r.length;o++)n=r[o],0<=e.indexOf(n)||(i[n]=t[n]);return i}(t,e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(t);for(o=0;o<r.length;o++)n=r[o],0<=e.indexOf(n)||Object.prototype.propertyIsEnumerable.call(t,n)&&(i[n]=t[n])}return i}function e(t){return function(t){if(Array.isArray(t)){for(var e=0,n=new Array(t.length);e<t.length;e++)n[e]=t[e];return n}}(t)||function(t){if(Symbol.iterator in Object(t)||"[object Arguments]"===Object.prototype.toString.call(t))return Array.from(t)}(t)||function(){throw new TypeError("Invalid attempt to spread non-iterable instance")}()}function t(t){if("undefined"!=typeof window&&window.navigator)return!!navigator.userAgent.match(t)}var w=t(/(?:Trident.*rv[ :]?11\.|msie|iemobile|Windows Phone)/i),E=t(/Edge/i),c=t(/firefox/i),s=t(/safari/i)&&!t(/chrome/i)&&!t(/android/i),n=t(/iP(ad|od|hone)/i),i=t(/chrome/i)&&t(/android/i),r={capture:!1,passive:!1};function u(t,e,n){t.addEventListener(e,n,!w&&r)}function d(t,e,n){t.removeEventListener(e,n,!w&&r)}function h(t,e){if(e){if(">"===e[0]&&(e=e.substring(1)),t)try{if(t.matches)return t.matches(e);if(t.msMatchesSelector)return t.msMatchesSelector(e);if(t.webkitMatchesSelector)return t.webkitMatchesSelector(e)}catch(t){return!1}return!1}}function P(t,e,n,o){if(t){n=n||document;do{if(null!=e&&(">"===e[0]?t.parentNode===n&&h(t,e):h(t,e))||o&&t===n)return t;if(t===n)break}while(t=(i=t).host&&i!==document&&i.host.nodeType?i.host:i.parentNode)}var i;return null}var f,p=/\s+/g;function k(t,e,n){if(t&&e)if(t.classList)t.classList[n?"add":"remove"](e);else{var o=(" "+t.className+" ").replace(p," ").replace(" "+e+" "," ");t.className=(o+(n?" "+e:"")).replace(p," ")}}function R(t,e,n){var o=t&&t.style;if(o){if(void 0===n)return document.defaultView&&document.defaultView.getComputedStyle?n=document.defaultView.getComputedStyle(t,""):t.currentStyle&&(n=t.currentStyle),void 0===e?n:n[e];e in o||-1!==e.indexOf("webkit")||(e="-webkit-"+e),o[e]=n+("string"==typeof n?"":"px")}}function v(t,e){var n="";if("string"==typeof t)n=t;else do{var o=R(t,"transform");o&&"none"!==o&&(n=o+" "+n)}while(!e&&(t=t.parentNode));var i=window.DOMMatrix||window.WebKitCSSMatrix||window.CSSMatrix||window.MSCSSMatrix;return i&&new i(n)}function g(t,e,n){if(t){var o=t.getElementsByTagName(e),i=0,r=o.length;if(n)for(;i<r;i++)n(o[i],i);return o}return[]}function N(){var t=document.scrollingElement;return t||document.documentElement}function X(t,e,n,o,i){if(t.getBoundingClientRect||t===window){var r,a,l,s,c,u,d;if(d=t!==window&&t!==N()?(a=(r=t.getBoundingClientRect()).top,l=r.left,s=r.bottom,c=r.right,u=r.height,r.width):(l=a=0,s=window.innerHeight,c=window.innerWidth,u=window.innerHeight,window.innerWidth),(e||n)&&t!==window&&(i=i||t.parentNode,!w))do{if(i&&i.getBoundingClientRect&&("none"!==R(i,"transform")||n&&"static"!==R(i,"position"))){var h=i.getBoundingClientRect();a-=h.top+parseInt(R(i,"border-top-width")),l-=h.left+parseInt(R(i,"border-left-width")),s=a+r.height,c=l+r.width;break}}while(i=i.parentNode);if(o&&t!==window){var f=v(i||t),p=f&&f.a,g=f&&f.d;f&&(s=(a/=g)+(u/=g),c=(l/=p)+(d/=p))}return{top:a,left:l,bottom:s,right:c,width:d,height:u}}}function Y(t,e,n){for(var o=H(t,!0),i=X(t)[e];o;){var r=X(o)[n];if(!("top"===n||"left"===n?r<=i:i<=r))return o;if(o===N())break;o=H(o,!1)}return!1}function m(t,e,n){for(var o=0,i=0,r=t.children;i<r.length;){if("none"!==r[i].style.display&&r[i]!==Rt.ghost&&r[i]!==Rt.dragged&&P(r[i],n.draggable,t,!1)){if(o===e)return r[i];o++}i++}return null}function B(t,e){for(var n=t.lastElementChild;n&&(n===Rt.ghost||"none"===R(n,"display")||e&&!h(n,e));)n=n.previousElementSibling;return n||null}function F(t,e){var n=0;if(!t||!t.parentNode)return-1;for(;t=t.previousElementSibling;)"TEMPLATE"===t.nodeName.toUpperCase()||t===Rt.clone||e&&!h(t,e)||n++;return n}function b(t){var e=0,n=0,o=N();if(t)do{var i=v(t),r=i.a,a=i.d;e+=t.scrollLeft*r,n+=t.scrollTop*a}while(t!==o&&(t=t.parentNode));return[e,n]}function H(t,e){if(!t||!t.getBoundingClientRect)return N();var n=t,o=!1;do{if(n.clientWidth<n.scrollWidth||n.clientHeight<n.scrollHeight){var i=R(n);if(n.clientWidth<n.scrollWidth&&("auto"==i.overflowX||"scroll"==i.overflowX)||n.clientHeight<n.scrollHeight&&("auto"==i.overflowY||"scroll"==i.overflowY)){if(!n.getBoundingClientRect||n===document.body)return N();if(o||e)return n;o=!0}}}while(n=n.parentNode);return N()}function y(t,e){return Math.round(t.top)===Math.round(e.top)&&Math.round(t.left)===Math.round(e.left)&&Math.round(t.height)===Math.round(e.height)&&Math.round(t.width)===Math.round(e.width)}function D(e,n){return function(){if(!f){var t=arguments;1===t.length?e.call(this,t[0]):e.apply(this,t),f=setTimeout(function(){f=void 0},n)}}}function L(t,e,n){t.scrollLeft+=e,t.scrollTop+=n}function S(t){var e=window.Polymer,n=window.jQuery||window.Zepto;return e&&e.dom?e.dom(t).cloneNode(!0):n?n(t).clone(!0)[0]:t.cloneNode(!0)}function _(t,e){R(t,"position","absolute"),R(t,"top",e.top),R(t,"left",e.left),R(t,"width",e.width),R(t,"height",e.height)}function C(t){R(t,"position",""),R(t,"top",""),R(t,"left",""),R(t,"width",""),R(t,"height","")}var j="Sortable"+(new Date).getTime();function T(){var e,o=[];return{captureAnimationState:function(){o=[],this.options.animation&&[].slice.call(this.el.children).forEach(function(t){if("none"!==R(t,"display")&&t!==Rt.ghost){o.push({target:t,rect:X(t)});var e=I({},o[o.length-1].rect);if(t.thisAnimationDuration){var n=v(t,!0);n&&(e.top-=n.f,e.left-=n.e)}t.fromRect=e}})},addAnimationState:function(t){o.push(t)},removeAnimationState:function(t){o.splice(function(t,e){for(var n in t)if(t.hasOwnProperty(n))for(var o in e)if(e.hasOwnProperty(o)&&e[o]===t[n][o])return Number(n);return-1}(o,{target:t}),1)},animateAll:function(t){var c=this;if(!this.options.animation)return clearTimeout(e),void("function"==typeof t&&t());var u=!1,d=0;o.forEach(function(t){var e=0,n=t.target,o=n.fromRect,i=X(n),r=n.prevFromRect,a=n.prevToRect,l=t.rect,s=v(n,!0);s&&(i.top-=s.f,i.left-=s.e),n.toRect=i,n.thisAnimationDuration&&y(r,i)&&!y(o,i)&&(l.top-i.top)/(l.left-i.left)==(o.top-i.top)/(o.left-i.left)&&(e=function(t,e,n,o){return Math.sqrt(Math.pow(e.top-t.top,2)+Math.pow(e.left-t.left,2))/Math.sqrt(Math.pow(e.top-n.top,2)+Math.pow(e.left-n.left,2))*o.animation}(l,r,a,c.options)),y(i,o)||(n.prevFromRect=o,n.prevToRect=i,e||(e=c.options.animation),c.animate(n,l,i,e)),e&&(u=!0,d=Math.max(d,e),clearTimeout(n.animationResetTimer),n.animationResetTimer=setTimeout(function(){n.animationTime=0,n.prevFromRect=null,n.fromRect=null,n.prevToRect=null,n.thisAnimationDuration=null},e),n.thisAnimationDuration=e)}),clearTimeout(e),u?e=setTimeout(function(){"function"==typeof t&&t()},d):"function"==typeof t&&t(),o=[]},animate:function(t,e,n,o){if(o){R(t,"transition",""),R(t,"transform","");var i=v(this.el),r=i&&i.a,a=i&&i.d,l=(e.left-n.left)/(r||1),s=(e.top-n.top)/(a||1);t.animatingX=!!l,t.animatingY=!!s,R(t,"transform","translate3d("+l+"px,"+s+"px,0)"),function(t){t.offsetWidth}(t),R(t,"transition","transform "+o+"ms"+(this.options.easing?" "+this.options.easing:"")),R(t,"transform","translate3d(0,0,0)"),"number"==typeof t.animated&&clearTimeout(t.animated),t.animated=setTimeout(function(){R(t,"transition",""),R(t,"transform",""),t.animated=!1,t.animatingX=!1,t.animatingY=!1},o)}}}}var x=[],M={initializeByDefault:!0},O={mount:function(t){for(var e in M)!M.hasOwnProperty(e)||e in t||(t[e]=M[e]);x.push(t)},pluginEvent:function(e,n,o){var t=this;this.eventCanceled=!1,o.cancel=function(){t.eventCanceled=!0};var i=e+"Global";x.forEach(function(t){n[t.pluginName]&&(n[t.pluginName][i]&&n[t.pluginName][i](I({sortable:n},o)),n.options[t.pluginName]&&n[t.pluginName][e]&&n[t.pluginName][e](I({sortable:n},o)))})},initializePlugins:function(o,i,r,t){for(var e in x.forEach(function(t){var e=t.pluginName;if(o.options[e]||t.initializeByDefault){var n=new t(o,i,o.options);n.sortable=o,n.options=o.options,o[e]=n,a(r,n.defaults)}}),o.options)if(o.options.hasOwnProperty(e)){var n=this.modifyOption(o,e,o.options[e]);void 0!==n&&(o.options[e]=n)}},getEventProperties:function(e,n){var o={};return x.forEach(function(t){"function"==typeof t.eventProperties&&a(o,t.eventProperties.call(n[t.pluginName],e))}),o},modifyOption:function(e,n,o){var i;return x.forEach(function(t){e[t.pluginName]&&t.optionListeners&&"function"==typeof t.optionListeners[n]&&(i=t.optionListeners[n].call(e[t.pluginName],o))}),i}};function A(t){var e=t.sortable,n=t.rootEl,o=t.name,i=t.targetEl,r=t.cloneEl,a=t.toEl,l=t.fromEl,s=t.oldIndex,c=t.newIndex,u=t.oldDraggableIndex,d=t.newDraggableIndex,h=t.originalEvent,f=t.putSortable,p=t.extraEventProperties;if(e=e||n&&n[j]){var g,v=e.options,m="on"+o.charAt(0).toUpperCase()+o.substr(1);!window.CustomEvent||w||E?(g=document.createEvent("Event")).initEvent(o,!0,!0):g=new CustomEvent(o,{bubbles:!0,cancelable:!0}),g.to=a||n,g.from=l||n,g.item=i||n,g.clone=r,g.oldIndex=s,g.newIndex=c,g.oldDraggableIndex=u,g.newDraggableIndex=d,g.originalEvent=h,g.pullMode=f?f.lastPutMode:void 0;var b=I({},p,O.getEventProperties(o,e));for(var y in b)g[y]=b[y];n&&n.dispatchEvent(g),v[m]&&v[m].call(e,g)}}function K(t,e,n){var o=2<arguments.length&&void 0!==n?n:{},i=o.evt,r=l(o,["evt"]);O.pluginEvent.bind(Rt)(t,e,I({dragEl:z,parentEl:G,ghostEl:U,rootEl:q,nextEl:V,lastDownEl:Z,cloneEl:Q,cloneHidden:$,dragStarted:dt,putSortable:it,activeSortable:Rt.active,originalEvent:i,oldIndex:J,oldDraggableIndex:et,newIndex:tt,newDraggableIndex:nt,hideGhostForTarget:Nt,unhideGhostForTarget:It,cloneNowHidden:function(){$=!0},cloneNowShown:function(){$=!1},dispatchSortableEvent:function(t){W({sortable:e,name:t,originalEvent:i})}},r))}function W(t){A(I({putSortable:it,cloneEl:Q,targetEl:z,rootEl:q,oldIndex:J,oldDraggableIndex:et,newIndex:tt,newDraggableIndex:nt},t))}var z,G,U,q,V,Z,Q,$,J,tt,et,nt,ot,it,rt,at,lt,st,ct,ut,dt,ht,ft,pt,gt,vt=!1,mt=!1,bt=[],yt=!1,wt=!1,Et=[],Dt=!1,St=[],_t="undefined"!=typeof document,Ct=n,Tt=E||w?"cssFloat":"float",xt=_t&&!i&&!n&&"draggable"in document.createElement("div"),Mt=function(){if(_t){if(w)return!1;var t=document.createElement("x");return t.style.cssText="pointer-events:auto","auto"===t.style.pointerEvents}}(),Ot=function(t,e){var n=R(t),o=parseInt(n.width)-parseInt(n.paddingLeft)-parseInt(n.paddingRight)-parseInt(n.borderLeftWidth)-parseInt(n.borderRightWidth),i=m(t,0,e),r=m(t,1,e),a=i&&R(i),l=r&&R(r),s=a&&parseInt(a.marginLeft)+parseInt(a.marginRight)+X(i).width,c=l&&parseInt(l.marginLeft)+parseInt(l.marginRight)+X(r).width;if("flex"===n.display)return"column"===n.flexDirection||"column-reverse"===n.flexDirection?"vertical":"horizontal";if("grid"===n.display)return n.gridTemplateColumns.split(" ").length<=1?"vertical":"horizontal";if(i&&a.float&&"none"!==a.float){var u="left"===a.float?"left":"right";return!r||"both"!==l.clear&&l.clear!==u?"horizontal":"vertical"}return i&&("block"===a.display||"flex"===a.display||"table"===a.display||"grid"===a.display||o<=s&&"none"===n[Tt]||r&&"none"===n[Tt]&&o<s+c)?"vertical":"horizontal"},At=function(t){function s(a,l){return function(t,e,n,o){var i=t.options.group.name&&e.options.group.name&&t.options.group.name===e.options.group.name;if(null==a&&(l||i))return!0;if(null==a||!1===a)return!1;if(l&&"clone"===a)return a;if("function"==typeof a)return s(a(t,e,n,o),l)(t,e,n,o);var r=(l?t:e).options.group.name;return!0===a||"string"==typeof a&&a===r||a.join&&-1<a.indexOf(r)}}var e={},n=t.group;n&&"object"==o(n)||(n={name:n}),e.name=n.name,e.checkPull=s(n.pull,!0),e.checkPut=s(n.put),e.revertClone=n.revertClone,t.group=e},Nt=function(){!Mt&&U&&R(U,"display","none")},It=function(){!Mt&&U&&R(U,"display","")};_t&&document.addEventListener("click",function(t){if(mt)return t.preventDefault(),t.stopPropagation&&t.stopPropagation(),t.stopImmediatePropagation&&t.stopImmediatePropagation(),mt=!1},!0);function Pt(t){if(z){var e=function(r,a){var l;return bt.some(function(t){if(!B(t)){var e=X(t),n=t[j].options.emptyInsertThreshold,o=r>=e.left-n&&r<=e.right+n,i=a>=e.top-n&&a<=e.bottom+n;return n&&o&&i?l=t:void 0}}),l}((t=t.touches?t.touches[0]:t).clientX,t.clientY);if(e){var n={};for(var o in t)t.hasOwnProperty(o)&&(n[o]=t[o]);n.target=n.rootEl=e,n.preventDefault=void 0,n.stopPropagation=void 0,e[j]._onDragOver(n)}}}function kt(t){z&&z.parentNode[j]._isOutsideThisEl(t.target)}function Rt(t,e){if(!t||!t.nodeType||1!==t.nodeType)throw"Sortable: `el` must be an HTMLElement, not ".concat({}.toString.call(t));this.el=t,this.options=e=a({},e),t[j]=this;var n={group:null,sort:!0,disabled:!1,store:null,handle:null,draggable:/^[uo]l$/i.test(t.nodeName)?">li":">*",swapThreshold:1,invertSwap:!1,invertedSwapThreshold:null,removeCloneOnHide:!0,direction:function(){return Ot(t,this.options)},ghostClass:"sortable-ghost",chosenClass:"sortable-chosen",dragClass:"sortable-drag",ignore:"a, img",filter:null,preventOnFilter:!0,animation:0,easing:null,setData:function(t,e){t.setData("Text",e.textContent)},dropBubble:!1,dragoverBubble:!1,dataIdAttr:"data-id",delay:0,delayOnTouchOnly:!1,touchStartThreshold:(Number.parseInt?Number:window).parseInt(window.devicePixelRatio,10)||1,forceFallback:!1,fallbackClass:"sortable-fallback",fallbackOnBody:!1,fallbackTolerance:0,fallbackOffset:{x:0,y:0},supportPointer:!1!==Rt.supportPointer&&"PointerEvent"in window,emptyInsertThreshold:5};for(var o in O.initializePlugins(this,t,n),n)o in e||(e[o]=n[o]);for(var i in At(e),this)"_"===i.charAt(0)&&"function"==typeof this[i]&&(this[i]=this[i].bind(this));this.nativeDraggable=!e.forceFallback&&xt,this.nativeDraggable&&(this.options.touchStartThreshold=1),e.supportPointer?u(t,"pointerdown",this._onTapStart):(u(t,"mousedown",this._onTapStart),u(t,"touchstart",this._onTapStart)),this.nativeDraggable&&(u(t,"dragover",this),u(t,"dragenter",this)),bt.push(this.el),e.store&&e.store.get&&this.sort(e.store.get(this)||[]),a(this,T())}function Xt(t,e,n,o,i,r,a,l){var s,c,u=t[j],d=u.options.onMove;return!window.CustomEvent||w||E?(s=document.createEvent("Event")).initEvent("move",!0,!0):s=new CustomEvent("move",{bubbles:!0,cancelable:!0}),s.to=e,s.from=t,s.dragged=n,s.draggedRect=o,s.related=i||e,s.relatedRect=r||X(e),s.willInsertAfter=l,s.originalEvent=a,t.dispatchEvent(s),d&&(c=d.call(u,s,a)),c}function Yt(t){t.draggable=!1}function Bt(){Dt=!1}function Ft(t){for(var e=t.tagName+t.className+t.src+t.href+t.textContent,n=e.length,o=0;n--;)o+=e.charCodeAt(n);return o.toString(36)}function Ht(t){return setTimeout(t,0)}function Lt(t){return clearTimeout(t)}Rt.prototype={constructor:Rt,_isOutsideThisEl:function(t){this.el.contains(t)||t===this.el||(ht=null)},_getDirection:function(t,e){return"function"==typeof this.options.direction?this.options.direction.call(this,t,e,z):this.options.direction},_onTapStart:function(e){if(e.cancelable){var n=this,o=this.el,t=this.options,i=t.preventOnFilter,r=e.type,a=e.touches&&e.touches[0]||e.pointerType&&"touch"===e.pointerType&&e,l=(a||e).target,s=e.target.shadowRoot&&(e.path&&e.path[0]||e.composedPath&&e.composedPath()[0])||l,c=t.filter;if(function(t){St.length=0;var e=t.getElementsByTagName("input"),n=e.length;for(;n--;){var o=e[n];o.checked&&St.push(o)}}(o),!z&&!(/mousedown|pointerdown/.test(r)&&0!==e.button||t.disabled||s.isContentEditable||(l=P(l,t.draggable,o,!1))&&l.animated||Z===l)){if(J=F(l),et=F(l,t.draggable),"function"==typeof c){if(c.call(this,e,l,this))return W({sortable:n,rootEl:s,name:"filter",targetEl:l,toEl:o,fromEl:o}),K("filter",n,{evt:e}),void(i&&e.cancelable&&e.preventDefault())}else if(c&&(c=c.split(",").some(function(t){if(t=P(s,t.trim(),o,!1))return W({sortable:n,rootEl:t,name:"filter",targetEl:l,fromEl:o,toEl:o}),K("filter",n,{evt:e}),!0})))return void(i&&e.cancelable&&e.preventDefault());t.handle&&!P(s,t.handle,o,!1)||this._prepareDragStart(e,a,l)}}},_prepareDragStart:function(t,e,n){var o,i=this,r=i.el,a=i.options,l=r.ownerDocument;if(n&&!z&&n.parentNode===r){var s=X(n);if(q=r,G=(z=n).parentNode,V=z.nextSibling,Z=n,ot=a.group,rt={target:Rt.dragged=z,clientX:(e||t).clientX,clientY:(e||t).clientY},ct=rt.clientX-s.left,ut=rt.clientY-s.top,this._lastX=(e||t).clientX,this._lastY=(e||t).clientY,z.style["will-change"]="all",o=function(){K("delayEnded",i,{evt:t}),Rt.eventCanceled?i._onDrop():(i._disableDelayedDragEvents(),!c&&i.nativeDraggable&&(z.draggable=!0),i._triggerDragStart(t,e),W({sortable:i,name:"choose",originalEvent:t}),k(z,a.chosenClass,!0))},a.ignore.split(",").forEach(function(t){g(z,t.trim(),Yt)}),u(l,"dragover",Pt),u(l,"mousemove",Pt),u(l,"touchmove",Pt),u(l,"mouseup",i._onDrop),u(l,"touchend",i._onDrop),u(l,"touchcancel",i._onDrop),c&&this.nativeDraggable&&(this.options.touchStartThreshold=4,z.draggable=!0),K("delayStart",this,{evt:t}),!a.delay||a.delayOnTouchOnly&&!e||this.nativeDraggable&&(E||w))o();else{if(Rt.eventCanceled)return void this._onDrop();u(l,"mouseup",i._disableDelayedDrag),u(l,"touchend",i._disableDelayedDrag),u(l,"touchcancel",i._disableDelayedDrag),u(l,"mousemove",i._delayedDragTouchMoveHandler),u(l,"touchmove",i._delayedDragTouchMoveHandler),a.supportPointer&&u(l,"pointermove",i._delayedDragTouchMoveHandler),i._dragStartTimer=setTimeout(o,a.delay)}}},_delayedDragTouchMoveHandler:function(t){var e=t.touches?t.touches[0]:t;Math.max(Math.abs(e.clientX-this._lastX),Math.abs(e.clientY-this._lastY))>=Math.floor(this.options.touchStartThreshold/(this.nativeDraggable&&window.devicePixelRatio||1))&&this._disableDelayedDrag()},_disableDelayedDrag:function(){z&&Yt(z),clearTimeout(this._dragStartTimer),this._disableDelayedDragEvents()},_disableDelayedDragEvents:function(){var t=this.el.ownerDocument;d(t,"mouseup",this._disableDelayedDrag),d(t,"touchend",this._disableDelayedDrag),d(t,"touchcancel",this._disableDelayedDrag),d(t,"mousemove",this._delayedDragTouchMoveHandler),d(t,"touchmove",this._delayedDragTouchMoveHandler),d(t,"pointermove",this._delayedDragTouchMoveHandler)},_triggerDragStart:function(t,e){e=e||"touch"==t.pointerType&&t,!this.nativeDraggable||e?this.options.supportPointer?u(document,"pointermove",this._onTouchMove):u(document,e?"touchmove":"mousemove",this._onTouchMove):(u(z,"dragend",this),u(q,"dragstart",this._onDragStart));try{document.selection?Ht(function(){document.selection.empty()}):window.getSelection().removeAllRanges()}catch(t){}},_dragStarted:function(t,e){if(vt=!1,q&&z){K("dragStarted",this,{evt:e}),this.nativeDraggable&&u(document,"dragover",kt);var n=this.options;t||k(z,n.dragClass,!1),k(z,n.ghostClass,!0),Rt.active=this,t&&this._appendGhost(),W({sortable:this,name:"start",originalEvent:e})}else this._nulling()},_emulateDragOver:function(){if(at){this._lastX=at.clientX,this._lastY=at.clientY,Nt();for(var t=document.elementFromPoint(at.clientX,at.clientY),e=t;t&&t.shadowRoot&&(t=t.shadowRoot.elementFromPoint(at.clientX,at.clientY))!==e;)e=t;if(z.parentNode[j]._isOutsideThisEl(t),e)do{if(e[j]){if(e[j]._onDragOver({clientX:at.clientX,clientY:at.clientY,target:t,rootEl:e})&&!this.options.dragoverBubble)break}t=e}while(e=e.parentNode);It()}},_onTouchMove:function(t){if(rt){var e=this.options,n=e.fallbackTolerance,o=e.fallbackOffset,i=t.touches?t.touches[0]:t,r=U&&v(U,!0),a=U&&r&&r.a,l=U&&r&&r.d,s=Ct&&gt&&b(gt),c=(i.clientX-rt.clientX+o.x)/(a||1)+(s?s[0]-Et[0]:0)/(a||1),u=(i.clientY-rt.clientY+o.y)/(l||1)+(s?s[1]-Et[1]:0)/(l||1);if(!Rt.active&&!vt){if(n&&Math.max(Math.abs(i.clientX-this._lastX),Math.abs(i.clientY-this._lastY))<n)return;this._onDragStart(t,!0)}if(U){r?(r.e+=c-(lt||0),r.f+=u-(st||0)):r={a:1,b:0,c:0,d:1,e:c,f:u};var d="matrix(".concat(r.a,",").concat(r.b,",").concat(r.c,",").concat(r.d,",").concat(r.e,",").concat(r.f,")");R(U,"webkitTransform",d),R(U,"mozTransform",d),R(U,"msTransform",d),R(U,"transform",d),lt=c,st=u,at=i}t.cancelable&&t.preventDefault()}},_appendGhost:function(){if(!U){var t=this.options.fallbackOnBody?document.body:q,e=X(z,!0,Ct,!0,t),n=this.options;if(Ct){for(gt=t;"static"===R(gt,"position")&&"none"===R(gt,"transform")&&gt!==document;)gt=gt.parentNode;gt!==document.body&&gt!==document.documentElement?(gt===document&&(gt=N()),e.top+=gt.scrollTop,e.left+=gt.scrollLeft):gt=N(),Et=b(gt)}k(U=z.cloneNode(!0),n.ghostClass,!1),k(U,n.fallbackClass,!0),k(U,n.dragClass,!0),R(U,"transition",""),R(U,"transform",""),R(U,"box-sizing","border-box"),R(U,"margin",0),R(U,"top",e.top),R(U,"left",e.left),R(U,"width",e.width),R(U,"height",e.height),R(U,"opacity","0.8"),R(U,"position",Ct?"absolute":"fixed"),R(U,"zIndex","100000"),R(U,"pointerEvents","none"),Rt.ghost=U,t.appendChild(U),R(U,"transform-origin",ct/parseInt(U.style.width)*100+"% "+ut/parseInt(U.style.height)*100+"%")}},_onDragStart:function(t,e){var n=this,o=t.dataTransfer,i=n.options;K("dragStart",this,{evt:t}),Rt.eventCanceled?this._onDrop():(K("setupClone",this),Rt.eventCanceled||((Q=S(z)).draggable=!1,Q.style["will-change"]="",this._hideClone(),k(Q,this.options.chosenClass,!1),Rt.clone=Q),n.cloneId=Ht(function(){K("clone",n),Rt.eventCanceled||(n.options.removeCloneOnHide||q.insertBefore(Q,z),n._hideClone(),W({sortable:n,name:"clone"}))}),e||k(z,i.dragClass,!0),e?(mt=!0,n._loopId=setInterval(n._emulateDragOver,50)):(d(document,"mouseup",n._onDrop),d(document,"touchend",n._onDrop),d(document,"touchcancel",n._onDrop),o&&(o.effectAllowed="move",i.setData&&i.setData.call(n,o,z)),u(document,"drop",n),R(z,"transform","translateZ(0)")),vt=!0,n._dragStartId=Ht(n._dragStarted.bind(n,e,t)),u(document,"selectstart",n),dt=!0,s&&R(document.body,"user-select","none"))},_onDragOver:function(n){var o,i,r,a,l=this.el,s=n.target,e=this.options,t=e.group,c=Rt.active,u=ot===t,d=e.sort,h=it||c,f=this,p=!1;if(!Dt){if(void 0!==n.preventDefault&&n.cancelable&&n.preventDefault(),s=P(s,e.draggable,l,!0),M("dragOver"),Rt.eventCanceled)return p;if(z.contains(n.target)||s.animated&&s.animatingX&&s.animatingY||f._ignoreWhileAnimating===s)return A(!1);if(mt=!1,c&&!e.disabled&&(u?d||(r=!q.contains(z)):it===this||(this.lastPutMode=ot.checkPull(this,c,z,n))&&t.checkPut(this,c,z,n))){if(a="vertical"===this._getDirection(n,s),o=X(z),M("dragOverValid"),Rt.eventCanceled)return p;if(r)return G=q,O(),this._hideClone(),M("revert"),Rt.eventCanceled||(V?q.insertBefore(z,V):q.appendChild(z)),A(!0);var g=B(l,e.draggable);if(!g||function(t,e,n){var o=X(B(n.el,n.options.draggable));return e?t.clientX>o.right+10||t.clientX<=o.right&&t.clientY>o.bottom&&t.clientX>=o.left:t.clientX>o.right&&t.clientY>o.top||t.clientX<=o.right&&t.clientY>o.bottom+10}(n,a,this)&&!g.animated){if(g===z)return A(!1);if(g&&l===n.target&&(s=g),s&&(i=X(s)),!1!==Xt(q,l,z,o,s,i,n,!!s))return O(),l.appendChild(z),G=l,N(),A(!0)}else if(s.parentNode===l){i=X(s);var v,m,b,y=z.parentNode!==l,w=!function(t,e,n){var o=n?t.left:t.top,i=n?t.right:t.bottom,r=n?t.width:t.height,a=n?e.left:e.top,l=n?e.right:e.bottom,s=n?e.width:e.height;return o===a||i===l||o+r/2===a+s/2}(z.animated&&z.toRect||o,s.animated&&s.toRect||i,a),E=a?"top":"left",D=Y(s,"top","top")||Y(z,"top","top"),S=D?D.scrollTop:void 0;if(ht!==s&&(m=i[E],yt=!1,wt=!w&&e.invertSwap||y),0!==(v=function(t,e,n,o,i,r,a,l){var s=o?t.clientY:t.clientX,c=o?n.height:n.width,u=o?n.top:n.left,d=o?n.bottom:n.right,h=!1;if(!a)if(l&&pt<c*i){if(!yt&&(1===ft?u+c*r/2<s:s<d-c*r/2)&&(yt=!0),yt)h=!0;else if(1===ft?s<u+pt:d-pt<s)return-ft}else if(u+c*(1-i)/2<s&&s<d-c*(1-i)/2)return function(t){return F(z)<F(t)?1:-1}(e);if((h=h||a)&&(s<u+c*r/2||d-c*r/2<s))return u+c/2<s?1:-1;return 0}(n,s,i,a,w?1:e.swapThreshold,null==e.invertedSwapThreshold?e.swapThreshold:e.invertedSwapThreshold,wt,ht===s)))for(var _=F(z);_-=v,(b=G.children[_])&&("none"===R(b,"display")||b===U););if(0===v||b===s)return A(!1);ft=v;var C=(ht=s).nextElementSibling,T=!1,x=Xt(q,l,z,o,s,i,n,T=1===v);if(!1!==x)return 1!==x&&-1!==x||(T=1===x),Dt=!0,setTimeout(Bt,30),O(),T&&!C?l.appendChild(z):s.parentNode.insertBefore(z,T?C:s),D&&L(D,0,S-D.scrollTop),G=z.parentNode,void 0===m||wt||(pt=Math.abs(m-X(s)[E])),N(),A(!0)}if(l.contains(z))return A(!1)}return!1}function M(t,e){K(t,f,I({evt:n,isOwner:u,axis:a?"vertical":"horizontal",revert:r,dragRect:o,targetRect:i,canSort:d,fromSortable:h,target:s,completed:A,onMove:function(t,e){return Xt(q,l,z,o,t,X(t),n,e)},changed:N},e))}function O(){M("dragOverAnimationCapture"),f.captureAnimationState(),f!==h&&h.captureAnimationState()}function A(t){return M("dragOverCompleted",{insertion:t}),t&&(u?c._hideClone():c._showClone(f),f!==h&&(k(z,it?it.options.ghostClass:c.options.ghostClass,!1),k(z,e.ghostClass,!0)),it!==f&&f!==Rt.active?it=f:f===Rt.active&&it&&(it=null),h===f&&(f._ignoreWhileAnimating=s),f.animateAll(function(){M("dragOverAnimationComplete"),f._ignoreWhileAnimating=null}),f!==h&&(h.animateAll(),h._ignoreWhileAnimating=null)),(s===z&&!z.animated||s===l&&!s.animated)&&(ht=null),e.dragoverBubble||n.rootEl||s===document||(z.parentNode[j]._isOutsideThisEl(n.target),t||Pt(n)),!e.dragoverBubble&&n.stopPropagation&&n.stopPropagation(),p=!0}function N(){tt=F(z),nt=F(z,e.draggable),W({sortable:f,name:"change",toEl:l,newIndex:tt,newDraggableIndex:nt,originalEvent:n})}},_ignoreWhileAnimating:null,_offMoveEvents:function(){d(document,"mousemove",this._onTouchMove),d(document,"touchmove",this._onTouchMove),d(document,"pointermove",this._onTouchMove),d(document,"dragover",Pt),d(document,"mousemove",Pt),d(document,"touchmove",Pt)},_offUpEvents:function(){var t=this.el.ownerDocument;d(t,"mouseup",this._onDrop),d(t,"touchend",this._onDrop),d(t,"pointerup",this._onDrop),d(t,"touchcancel",this._onDrop),d(document,"selectstart",this)},_onDrop:function(t){var e=this.el,n=this.options;tt=F(z),nt=F(z,n.draggable),K("drop",this,{evt:t}),G=z&&z.parentNode,tt=F(z),nt=F(z,n.draggable),Rt.eventCanceled||(yt=wt=vt=!1,clearInterval(this._loopId),clearTimeout(this._dragStartTimer),Lt(this.cloneId),Lt(this._dragStartId),this.nativeDraggable&&(d(document,"drop",this),d(e,"dragstart",this._onDragStart)),this._offMoveEvents(),this._offUpEvents(),s&&R(document.body,"user-select",""),R(z,"transform",""),t&&(dt&&(t.cancelable&&t.preventDefault(),n.dropBubble||t.stopPropagation()),U&&U.parentNode&&U.parentNode.removeChild(U),(q===G||it&&"clone"!==it.lastPutMode)&&Q&&Q.parentNode&&Q.parentNode.removeChild(Q),z&&(this.nativeDraggable&&d(z,"dragend",this),Yt(z),z.style["will-change"]="",dt&&!vt&&k(z,it?it.options.ghostClass:this.options.ghostClass,!1),k(z,this.options.chosenClass,!1),W({sortable:this,name:"unchoose",toEl:G,newIndex:null,newDraggableIndex:null,originalEvent:t}),q!==G?(0<=tt&&(W({rootEl:G,name:"add",toEl:G,fromEl:q,originalEvent:t}),W({sortable:this,name:"remove",toEl:G,originalEvent:t}),W({rootEl:G,name:"sort",toEl:G,fromEl:q,originalEvent:t}),W({sortable:this,name:"sort",toEl:G,originalEvent:t})),it&&it.save()):tt!==J&&0<=tt&&(W({sortable:this,name:"update",toEl:G,originalEvent:t}),W({sortable:this,name:"sort",toEl:G,originalEvent:t})),Rt.active&&(null!=tt&&-1!==tt||(tt=J,nt=et),W({sortable:this,name:"end",toEl:G,originalEvent:t}),this.save())))),this._nulling()},_nulling:function(){K("nulling",this),q=z=G=U=V=Q=Z=$=rt=at=dt=tt=nt=J=et=ht=ft=it=ot=Rt.dragged=Rt.ghost=Rt.clone=Rt.active=null,St.forEach(function(t){t.checked=!0}),St.length=lt=st=0},handleEvent:function(t){switch(t.type){case"drop":case"dragend":this._onDrop(t);break;case"dragenter":case"dragover":z&&(this._onDragOver(t),function(t){t.dataTransfer&&(t.dataTransfer.dropEffect="move");t.cancelable&&t.preventDefault()}(t));break;case"selectstart":t.preventDefault()}},toArray:function(){for(var t,e=[],n=this.el.children,o=0,i=n.length,r=this.options;o<i;o++)P(t=n[o],r.draggable,this.el,!1)&&e.push(t.getAttribute(r.dataIdAttr)||Ft(t));return e},sort:function(t){var o={},i=this.el;this.toArray().forEach(function(t,e){var n=i.children[e];P(n,this.options.draggable,i,!1)&&(o[t]=n)},this),t.forEach(function(t){o[t]&&(i.removeChild(o[t]),i.appendChild(o[t]))})},save:function(){var t=this.options.store;t&&t.set&&t.set(this)},closest:function(t,e){return P(t,e||this.options.draggable,this.el,!1)},option:function(t,e){var n=this.options;if(void 0===e)return n[t];var o=O.modifyOption(this,t,e);n[t]=void 0!==o?o:e,"group"===t&&At(n)},destroy:function(){K("destroy",this);var t=this.el;t[j]=null,d(t,"mousedown",this._onTapStart),d(t,"touchstart",this._onTapStart),d(t,"pointerdown",this._onTapStart),this.nativeDraggable&&(d(t,"dragover",this),d(t,"dragenter",this)),Array.prototype.forEach.call(t.querySelectorAll("[draggable]"),function(t){t.removeAttribute("draggable")}),this._onDrop(),this._disableDelayedDragEvents(),bt.splice(bt.indexOf(this.el),1),this.el=t=null},_hideClone:function(){if(!$){if(K("hideClone",this),Rt.eventCanceled)return;R(Q,"display","none"),this.options.removeCloneOnHide&&Q.parentNode&&Q.parentNode.removeChild(Q),$=!0}},_showClone:function(t){if("clone"===t.lastPutMode){if($){if(K("showClone",this),Rt.eventCanceled)return;q.contains(z)&&!this.options.group.revertClone?q.insertBefore(Q,z):V?q.insertBefore(Q,V):q.appendChild(Q),this.options.group.revertClone&&this.animate(z,Q),R(Q,"display",""),$=!1}}else this._hideClone()}},_t&&u(document,"touchmove",function(t){(Rt.active||vt)&&t.cancelable&&t.preventDefault()}),Rt.utils={on:u,off:d,css:R,find:g,is:function(t,e){return!!P(t,e,t,!1)},extend:function(t,e){if(t&&e)for(var n in e)e.hasOwnProperty(n)&&(t[n]=e[n]);return t},throttle:D,closest:P,toggleClass:k,clone:S,index:F,nextTick:Ht,cancelNextTick:Lt,detectDirection:Ot,getChild:m},Rt.get=function(t){return t[j]},Rt.mount=function(){for(var t=arguments.length,e=new Array(t),n=0;n<t;n++)e[n]=arguments[n];e[0].constructor===Array&&(e=e[0]),e.forEach(function(t){if(!t.prototype||!t.prototype.constructor)throw"Sortable: Mounted plugin must be a constructor function, not ".concat({}.toString.call(t));t.utils&&(Rt.utils=I({},Rt.utils,t.utils)),O.mount(t)})},Rt.create=function(t,e){return new Rt(t,e)};var jt,Kt,Wt,zt,Gt,Ut,qt=[],Vt=!(Rt.version="1.10.2");function Zt(){qt.forEach(function(t){clearInterval(t.pid)}),qt=[]}function Qt(){clearInterval(Ut)}function $t(t){var e=t.originalEvent,n=t.putSortable,o=t.dragEl,i=t.activeSortable,r=t.dispatchSortableEvent,a=t.hideGhostForTarget,l=t.unhideGhostForTarget;if(e){var s=n||i;a();var c=e.changedTouches&&e.changedTouches.length?e.changedTouches[0]:e,u=document.elementFromPoint(c.clientX,c.clientY);l(),s&&!s.el.contains(u)&&(r("spill"),this.onSpill({dragEl:o,putSortable:n}))}}var Jt,te=D(function(n,t,e,o){if(t.scroll){var i,r=(n.touches?n.touches[0]:n).clientX,a=(n.touches?n.touches[0]:n).clientY,l=t.scrollSensitivity,s=t.scrollSpeed,c=N(),u=!1;Kt!==e&&(Kt=e,Zt(),jt=t.scroll,i=t.scrollFn,!0===jt&&(jt=H(e,!0)));var d=0,h=jt;do{var f=h,p=X(f),g=p.top,v=p.bottom,m=p.left,b=p.right,y=p.width,w=p.height,E=void 0,D=void 0,S=f.scrollWidth,_=f.scrollHeight,C=R(f),T=f.scrollLeft,x=f.scrollTop;D=f===c?(E=y<S&&("auto"===C.overflowX||"scroll"===C.overflowX||"visible"===C.overflowX),w<_&&("auto"===C.overflowY||"scroll"===C.overflowY||"visible"===C.overflowY)):(E=y<S&&("auto"===C.overflowX||"scroll"===C.overflowX),w<_&&("auto"===C.overflowY||"scroll"===C.overflowY));var M=E&&(Math.abs(b-r)<=l&&T+y<S)-(Math.abs(m-r)<=l&&!!T),O=D&&(Math.abs(v-a)<=l&&x+w<_)-(Math.abs(g-a)<=l&&!!x);if(!qt[d])for(var A=0;A<=d;A++)qt[A]||(qt[A]={});qt[d].vx==M&&qt[d].vy==O&&qt[d].el===f||(qt[d].el=f,qt[d].vx=M,qt[d].vy=O,clearInterval(qt[d].pid),0==M&&0==O||(u=!0,qt[d].pid=setInterval(function(){o&&0===this.layer&&Rt.active._onTouchMove(Gt);var t=qt[this.layer].vy?qt[this.layer].vy*s:0,e=qt[this.layer].vx?qt[this.layer].vx*s:0;"function"==typeof i&&"continue"!==i.call(Rt.dragged.parentNode[j],e,t,n,Gt,qt[this.layer].el)||L(qt[this.layer].el,e,t)}.bind({layer:d}),24))),d++}while(t.bubbleScroll&&h!==c&&(h=H(h,!1)));Vt=u}},30);function ee(){}function ne(){}ee.prototype={startIndex:null,dragStart:function(t){var e=t.oldDraggableIndex;this.startIndex=e},onSpill:function(t){var e=t.dragEl,n=t.putSortable;this.sortable.captureAnimationState(),n&&n.captureAnimationState();var o=m(this.sortable.el,this.startIndex,this.options);o?this.sortable.el.insertBefore(e,o):this.sortable.el.appendChild(e),this.sortable.animateAll(),n&&n.animateAll()},drop:$t},a(ee,{pluginName:"revertOnSpill"}),ne.prototype={onSpill:function(t){var e=t.dragEl,n=t.putSortable||this.sortable;n.captureAnimationState(),e.parentNode&&e.parentNode.removeChild(e),n.animateAll()},drop:$t},a(ne,{pluginName:"removeOnSpill"});var oe,ie,re,ae,le,se=[],ce=[],ue=!1,de=!1,he=!1;function fe(o,i){ce.forEach(function(t,e){var n=i.children[t.sortableIndex+(o?Number(e):0)];n?i.insertBefore(t,n):i.appendChild(t)})}function pe(){se.forEach(function(t){t!==re&&t.parentNode&&t.parentNode.removeChild(t)})}return Rt.mount(new function(){function t(){for(var t in this.defaults={scroll:!0,scrollSensitivity:30,scrollSpeed:10,bubbleScroll:!0},this)"_"===t.charAt(0)&&"function"==typeof this[t]&&(this[t]=this[t].bind(this))}return t.prototype={dragStarted:function(t){var e=t.originalEvent;this.sortable.nativeDraggable?u(document,"dragover",this._handleAutoScroll):this.options.supportPointer?u(document,"pointermove",this._handleFallbackAutoScroll):e.touches?u(document,"touchmove",this._handleFallbackAutoScroll):u(document,"mousemove",this._handleFallbackAutoScroll)},dragOverCompleted:function(t){var e=t.originalEvent;this.options.dragOverBubble||e.rootEl||this._handleAutoScroll(e)},drop:function(){this.sortable.nativeDraggable?d(document,"dragover",this._handleAutoScroll):(d(document,"pointermove",this._handleFallbackAutoScroll),d(document,"touchmove",this._handleFallbackAutoScroll),d(document,"mousemove",this._handleFallbackAutoScroll)),Qt(),Zt(),clearTimeout(f),f=void 0},nulling:function(){Gt=Kt=jt=Vt=Ut=Wt=zt=null,qt.length=0},_handleFallbackAutoScroll:function(t){this._handleAutoScroll(t,!0)},_handleAutoScroll:function(e,n){var o=this,i=(e.touches?e.touches[0]:e).clientX,r=(e.touches?e.touches[0]:e).clientY,t=document.elementFromPoint(i,r);if(Gt=e,n||E||w||s){te(e,this.options,t,n);var a=H(t,!0);!Vt||Ut&&i===Wt&&r===zt||(Ut&&Qt(),Ut=setInterval(function(){var t=H(document.elementFromPoint(i,r),!0);t!==a&&(a=t,Zt()),te(e,o.options,t,n)},10),Wt=i,zt=r)}else{if(!this.options.bubbleScroll||H(t,!0)===N())return void Zt();te(e,this.options,H(t,!1),!1)}}},a(t,{pluginName:"scroll",initializeByDefault:!0})}),Rt.mount(ne,ee),Rt.mount(new function(){function t(){this.defaults={swapClass:"sortable-swap-highlight"}}return t.prototype={dragStart:function(t){var e=t.dragEl;Jt=e},dragOverValid:function(t){var e=t.completed,n=t.target,o=t.onMove,i=t.activeSortable,r=t.changed,a=t.cancel;if(i.options.swap){var l=this.sortable.el,s=this.options;if(n&&n!==l){var c=Jt;Jt=!1!==o(n)?(k(n,s.swapClass,!0),n):null,c&&c!==Jt&&k(c,s.swapClass,!1)}r(),e(!0),a()}},drop:function(t){var e=t.activeSortable,n=t.putSortable,o=t.dragEl,i=n||this.sortable,r=this.options;Jt&&k(Jt,r.swapClass,!1),Jt&&(r.swap||n&&n.options.swap)&&o!==Jt&&(i.captureAnimationState(),i!==e&&e.captureAnimationState(),function(t,e){var n,o,i=t.parentNode,r=e.parentNode;if(!i||!r||i.isEqualNode(e)||r.isEqualNode(t))return;n=F(t),o=F(e),i.isEqualNode(r)&&n<o&&o++;i.insertBefore(e,i.children[n]),r.insertBefore(t,r.children[o])}(o,Jt),i.animateAll(),i!==e&&e.animateAll())},nulling:function(){Jt=null}},a(t,{pluginName:"swap",eventProperties:function(){return{swapItem:Jt}}})}),Rt.mount(new function(){function t(o){for(var t in this)"_"===t.charAt(0)&&"function"==typeof this[t]&&(this[t]=this[t].bind(this));o.options.supportPointer?u(document,"pointerup",this._deselectMultiDrag):(u(document,"mouseup",this._deselectMultiDrag),u(document,"touchend",this._deselectMultiDrag)),u(document,"keydown",this._checkKeyDown),u(document,"keyup",this._checkKeyUp),this.defaults={selectedClass:"sortable-selected",multiDragKey:null,setData:function(t,e){var n="";se.length&&ie===o?se.forEach(function(t,e){n+=(e?", ":"")+t.textContent}):n=e.textContent,t.setData("Text",n)}}}return t.prototype={multiDragKeyDown:!1,isMultiDrag:!1,delayStartGlobal:function(t){var e=t.dragEl;re=e},delayEnded:function(){this.isMultiDrag=~se.indexOf(re)},setupClone:function(t){var e=t.sortable,n=t.cancel;if(this.isMultiDrag){for(var o=0;o<se.length;o++)ce.push(S(se[o])),ce[o].sortableIndex=se[o].sortableIndex,ce[o].draggable=!1,ce[o].style["will-change"]="",k(ce[o],this.options.selectedClass,!1),se[o]===re&&k(ce[o],this.options.chosenClass,!1);e._hideClone(),n()}},clone:function(t){var e=t.sortable,n=t.rootEl,o=t.dispatchSortableEvent,i=t.cancel;this.isMultiDrag&&(this.options.removeCloneOnHide||se.length&&ie===e&&(fe(!0,n),o("clone"),i()))},showClone:function(t){var e=t.cloneNowShown,n=t.rootEl,o=t.cancel;this.isMultiDrag&&(fe(!1,n),ce.forEach(function(t){R(t,"display","")}),e(),le=!1,o())},hideClone:function(t){var e=this,n=(t.sortable,t.cloneNowHidden),o=t.cancel;this.isMultiDrag&&(ce.forEach(function(t){R(t,"display","none"),e.options.removeCloneOnHide&&t.parentNode&&t.parentNode.removeChild(t)}),n(),le=!0,o())},dragStartGlobal:function(t){t.sortable;!this.isMultiDrag&&ie&&ie.multiDrag._deselectMultiDrag(),se.forEach(function(t){t.sortableIndex=F(t)}),se=se.sort(function(t,e){return t.sortableIndex-e.sortableIndex}),he=!0},dragStarted:function(t){var e=this,n=t.sortable;if(this.isMultiDrag){if(this.options.sort&&(n.captureAnimationState(),this.options.animation)){se.forEach(function(t){t!==re&&R(t,"position","absolute")});var o=X(re,!1,!0,!0);se.forEach(function(t){t!==re&&_(t,o)}),ue=de=!0}n.animateAll(function(){ue=de=!1,e.options.animation&&se.forEach(function(t){C(t)}),e.options.sort&&pe()})}},dragOver:function(t){var e=t.target,n=t.completed,o=t.cancel;de&&~se.indexOf(e)&&(n(!1),o())},revert:function(t){var e=t.fromSortable,n=t.rootEl,o=t.sortable,i=t.dragRect;1<se.length&&(se.forEach(function(t){o.addAnimationState({target:t,rect:de?X(t):i}),C(t),t.fromRect=i,e.removeAnimationState(t)}),de=!1,function(o,i){se.forEach(function(t,e){var n=i.children[t.sortableIndex+(o?Number(e):0)];n?i.insertBefore(t,n):i.appendChild(t)})}(!this.options.removeCloneOnHide,n))},dragOverCompleted:function(t){var e=t.sortable,n=t.isOwner,o=t.insertion,i=t.activeSortable,r=t.parentEl,a=t.putSortable,l=this.options;if(o){if(n&&i._hideClone(),ue=!1,l.animation&&1<se.length&&(de||!n&&!i.options.sort&&!a)){var s=X(re,!1,!0,!0);se.forEach(function(t){t!==re&&(_(t,s),r.appendChild(t))}),de=!0}if(!n)if(de||pe(),1<se.length){var c=le;i._showClone(e),i.options.animation&&!le&&c&&ce.forEach(function(t){i.addAnimationState({target:t,rect:ae}),t.fromRect=ae,t.thisAnimationDuration=null})}else i._showClone(e)}},dragOverAnimationCapture:function(t){var e=t.dragRect,n=t.isOwner,o=t.activeSortable;if(se.forEach(function(t){t.thisAnimationDuration=null}),o.options.animation&&!n&&o.multiDrag.isMultiDrag){ae=a({},e);var i=v(re,!0);ae.top-=i.f,ae.left-=i.e}},dragOverAnimationComplete:function(){de&&(de=!1,pe())},drop:function(t){var e=t.originalEvent,n=t.rootEl,o=t.parentEl,i=t.sortable,r=t.dispatchSortableEvent,a=t.oldIndex,l=t.putSortable,s=l||this.sortable;if(e){var c=this.options,u=o.children;if(!he)if(c.multiDragKey&&!this.multiDragKeyDown&&this._deselectMultiDrag(),k(re,c.selectedClass,!~se.indexOf(re)),~se.indexOf(re))se.splice(se.indexOf(re),1),oe=null,A({sortable:i,rootEl:n,name:"deselect",targetEl:re,originalEvt:e});else{if(se.push(re),A({sortable:i,rootEl:n,name:"select",targetEl:re,originalEvt:e}),e.shiftKey&&oe&&i.el.contains(oe)){var d,h,f=F(oe),p=F(re);if(~f&&~p&&f!==p)for(d=f<p?(h=f,p):(h=p,f+1);h<d;h++)~se.indexOf(u[h])||(k(u[h],c.selectedClass,!0),se.push(u[h]),A({sortable:i,rootEl:n,name:"select",targetEl:u[h],originalEvt:e}))}else oe=re;ie=s}if(he&&this.isMultiDrag){if((o[j].options.sort||o!==n)&&1<se.length){var g=X(re),v=F(re,":not(."+this.options.selectedClass+")");if(!ue&&c.animation&&(re.thisAnimationDuration=null),s.captureAnimationState(),!ue&&(c.animation&&(re.fromRect=g,se.forEach(function(t){if(t.thisAnimationDuration=null,t!==re){var e=de?X(t):g;t.fromRect=e,s.addAnimationState({target:t,rect:e})}})),pe(),se.forEach(function(t){u[v]?o.insertBefore(t,u[v]):o.appendChild(t),v++}),a===F(re))){var m=!1;se.forEach(function(t){t.sortableIndex===F(t)||(m=!0)}),m&&r("update")}se.forEach(function(t){C(t)}),s.animateAll()}ie=s}(n===o||l&&"clone"!==l.lastPutMode)&&ce.forEach(function(t){t.parentNode&&t.parentNode.removeChild(t)})}},nullingGlobal:function(){this.isMultiDrag=he=!1,ce.length=0},destroyGlobal:function(){this._deselectMultiDrag(),d(document,"pointerup",this._deselectMultiDrag),d(document,"mouseup",this._deselectMultiDrag),d(document,"touchend",this._deselectMultiDrag),d(document,"keydown",this._checkKeyDown),d(document,"keyup",this._checkKeyUp)},_deselectMultiDrag:function(t){if(!(void 0!==he&&he||ie!==this.sortable||t&&P(t.target,this.options.draggable,this.sortable.el,!1)||t&&0!==t.button))for(;se.length;){var e=se[0];k(e,this.options.selectedClass,!1),se.shift(),A({sortable:this.sortable,rootEl:this.sortable.el,name:"deselect",targetEl:e,originalEvt:t})}},_checkKeyDown:function(t){t.key===this.options.multiDragKey&&(this.multiDragKeyDown=!0)},_checkKeyUp:function(t){t.key===this.options.multiDragKey&&(this.multiDragKeyDown=!1)}},a(t,{pluginName:"multiDrag",utils:{select:function(t){var e=t.parentNode[j];e&&e.options.multiDrag&&!~se.indexOf(t)&&(ie&&ie!==e&&(ie.multiDrag._deselectMultiDrag(),ie=e),k(t,e.options.selectedClass,!0),se.push(t))},deselect:function(t){var e=t.parentNode[j],n=se.indexOf(t);e&&e.options.multiDrag&&~n&&(k(t,e.options.selectedClass,!1),se.splice(n,1))}},eventProperties:function(){var n=this,o=[],i=[];return se.forEach(function(t){var e;o.push({multiDragElement:t,index:t.sortableIndex}),e=de&&t!==re?-1:de?F(t,":not(."+n.options.selectedClass+")"):F(t),i.push({multiDragElement:t,index:e})}),{items:e(se),clones:[].concat(ce),oldIndicies:o,newIndicies:i}},optionListeners:{multiDragKey:function(t){return"ctrl"===(t=t.toLowerCase())?t="Control":1<t.length&&(t=t.charAt(0).toUpperCase()+t.substr(1)),t}}})}),Rt});
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
 * Apply a response curve to a 0-max value.
 * Must match the C++ applyCurve() in MacroTranslator.cpp exactly.
 * For 14-bit NRPN: normalizes to 0-127, applies curve, scales back. Integer-only.
 * Must match the C++ applyCurve() in MacroTranslator.cpp exactly.
 */
function applyCurve(val, curveType, max) {
  max = max || 127;
  if (!curveType || curveType === 'linear') return val;
  if (val <= 0) return 0;
  if (val >= max) return max;

  // Normalize to 0-127 for curve computation
  var n = Math.round(val * 127 / max);
  var curved;

  switch (curveType) {
    case 'log':
      if (n <= 16) curved = n * 4;
      else if (n <= 64) curved = 64 + Math.round((n - 16) * 36 / 48);
      else curved = 100 + Math.round((n - 64) * 27 / 63);
      break;

    case 'exp':
      curved = Math.round(n * n / 127);
      break;

    default:
      curved = n;
      break;
  }

  // Scale back to original range
  return Math.round(curved * max / 127);
}

/**
 * Compute the real CC/NRPN output values for a given knob value.
 * Returns an array of { ctrl, name, value, pct, max, type } for each mapping target.
 *   ctrl  — CC/NRPN number
 *   name  — human-readable DSP param name
 *   value — computed output (0-127 for CC, 0-16383 for NRPN)
 *   pct   — percentage of max (for bar display)
 *   max   — 127 or 16383
 *   type  — 'cc' or 'nrpm'
 */
function computeMappingOutputs(def, paramIdx, knobValue) {
  if (!def || !def.mapping) return [];
  var results = [];
  def.mapping.forEach(function(m) {
    if (!m.add) return;
    m.add.forEach(function(a) {
      if (a.src !== paramIdx) return;
      var max = getParamMax(def.machine, m.ctrl);
      var paramInfo = getParamInfo(def.machine, m.ctrl);
      var type = (paramInfo && paramInfo.type) || 'cc';
      var curved = applyCurve(knobValue, a.curve, max);
      var val = (m.start || 0) + Math.round(curved * a.mul / a.div);
      val = Math.max(0, Math.min(max, val));
      results.push({
        ctrl: m.ctrl,
        name: resolveCCName(def.machine, m.ctrl),
        value: val,
        pct: Math.round(val / max * 100),
        max: max,
        type: type
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
  trackDefaults: null,
  activeTrack: -1,
  loaded: false,
};

var _trackChangeCallbacks = [];

/**
 * Load all data from the device (synthdefs, macrodefs, soundpresets, tracks).
 * Called once at boot; both views read from sharedData.
 *
 * Uses TWO sequential requests to stay within ESP32 HTTP socket limits:
 *   1. GET /api/v2/storage?getconfig=synthdefinitions.json  → synth defs
 *   2. GET /api/v2/macros?action=getall                     → bulk macro data
 *
 * The "getall" endpoint returns { macroDefs, soundPresets, tracks } in a
 * single response, replacing the previous 60+ individual file-fetches.
 */
function loadSharedData() {
  showLoading('Loading tracks & definitions…');
  return fetch('/api/v2/storage?getconfig=synthdefinitions.json').then(function(r) {
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
    // Fetch track defaults (boot presets) — non-critical, so failures are tolerated
    return fetch('/api/v2/macros?action=get_trackdefaults').then(function(r) {
      return r.ok ? r.json() : null;
    }).then(function(tdData) {
      sharedData.trackDefaults = tdData && tdData.tracks ? tdData : { tracks: [] };
    }).catch(function() {
      sharedData.trackDefaults = { tracks: [] };
    });
  }).then(function() {
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
function reloadFirmwareMacros(defId) {
  var url = '/api/v2/macros?action=reload';
  if (defId) url += '&id=' + encodeURIComponent(defId);
  return fetch(url, { method: 'POST' })
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
 * Look up a parameter's type info from the machine definition.
 * Returns the parameter object { ctrl, type, name, ... } or null.
 */
function getParamInfo(machineId, ctrl) {
  var info = getMachineInfo(machineId);
  if (!info || !info.parameters) return null;
  return info.parameters.find(function(p) { return p.ctrl === ctrl; }) || null;
}

/**
 * Get the max value for a parameter based on its type.
 * NRPN (type: 'nrpm') → 16383, CC → 127.
 */
function getParamMax(machineId, ctrl) {
  var param = getParamInfo(machineId, ctrl);
  return (param && param.type === 'nrpm') ? 16383 : 127;
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
    if (track.type === 'drum') classes += ' track-drum';
    else if (track.type === 'synth') classes += ' track-synth';
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
          var oMax = o.max || 127;
          var rangeLow = 0, rangeHigh = oMax;
          var sourceCurve = '';
          if (mapping && mapping.add) {
            if (mapping.add.length === 1) {
              rangeLow = mapping.start || 0;
              var a = mapping.add[0];
              rangeHigh = rangeLow + Math.round(oMax * (a.mul || 1) / (a.div || 1));
              rangeHigh = Math.min(oMax, rangeHigh);
              sourceCurve = a.curve || '';
            } else {
              rangeLow = mapping.start || 0;
              rangeHigh = rangeLow;
              mapping.add.forEach(function(a) {
                rangeHigh += Math.round(oMax * (a.mul || 1) / (a.div || 1));
                if (a.src === param.idx) sourceCurve = a.curve || '';
              });
              rangeHigh = Math.min(oMax, rangeHigh);
            }
          }
          var rangeLowPct = rangeLow / oMax * 100;
          var rangeWidthPct = (rangeHigh - rangeLow) / oMax * 100;
          var valuePct = o.value / oMax * 100;

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
              var physVal = targetDH.rawToDisplay(o.value, 0, oMax, targetHint);
              targetFmt = targetDH.formatDisplayValue(physVal, targetHint);
            }
          }
          html += '<span class="knob-target-val">' + targetFmt + '</span>';
          // 14-bit badge
          if (mapping && (mapping.bits === 14 || mapping.type === 'nrpm')) {
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
  getParamInfo: getParamInfo,
  getParamMax: getParamMax,
  getTrackMachines: getTrackMachines,
  renderTrackOverview: renderTrackOverview,
  setupTrackOverviewEvents: setupTrackOverviewEvents,
  setActiveTab: setActiveTab,
  getActiveTab: getActiveTab,
};

// ── factory-manifest.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Factory Manifest
//
// Read-only registry of factory macro definitions and sound presets
// shipped with the device. Factory items cannot be overwritten or
// deleted — users may only clone/save-as with a new name.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var FACTORY_DEFINITIONS = [
    'ab-allparams',
    'as-allparams',
    'cl-allparams',
    'db-allparams',
    'db-phatpunch',
    'db-submorph',
    'ds-allparams',
    'ds-snappy',
    'extdrum-allparams',
    'extsynth-allparams',
    'fmb-allparams',
    'fmb-deepfm',
    'fmb-metallic',
    'fxdelay-allparams',
    'fxmaster-allparams',
    'fxreverb-allparams',
    'hh1-allparams',
    'hh2-allparams',
    'inp-allparams',
    'mo-allparams',
    'nodrum-allparams',
    'nofx-allparams',
    'nosynth-allparams',
    'pp-allparams',
    'pp-darkchord',
    'pp-lushpad',
    'ro-allparams',
    'ro-fullrompler',
    'rs-allparams',
    'td3-acidbass',
    'td3-allparams',
    'wtosc-allparams',
    'wtosc-morphpad',
  ];

  var FACTORY_PRESETS = [
    'ab-all-def',
    'as-all-def',
    'cl-all-def',
    'db-all-def',
    'ds-all-def',
    'ds-snap1',
    'extdrum-all-def',
    'extsynth-all-def',
    'fmb-all-def',
    'fxdelay-all-def',
    'fxmaster-all-def',
    'fxreverb-all-def',
    'golem',
    'hh1-all-def',
    'hh2-all-def',
    'inp-all-def',
    'mo-all-def',
    'msp-acidbass1',
    'msp-acidbass2',
    'msp-darkchord1',
    'msp-darkchord2',
    'msp-deepfm1',
    'msp-deepfm2',
    'msp-lushpad1',
    'msp-lushpad2',
    'msp-metallic1',
    'msp-morphpad1',
    'msp-morphpad2',
    'msp-phatpunch1',
    'msp-phatpunch2',
    'msp-submorph1',
    'new-kick',
    'nodrum-all-def',
    'nofx-all-def',
    'nosynth-all-def',
    'pp-all-def',
    'punch-testing',
    'ro-all-def',
    'ro-fullrompler-def',
    'rs-all-def',
    'sub-morphing-preset-',
    'td3-all-def',
    'wtosc-all-def',
  ];

  // Build lookup sets for O(1) checks
  var defSet = {};
  FACTORY_DEFINITIONS.forEach(function(id) { defSet[id] = true; });
  var presetSet = {};
  FACTORY_PRESETS.forEach(function(id) { presetSet[id] = true; });

  // Factory edit unlock state — persisted across pages via sessionStorage
  var _unlocked = sessionStorage.getItem('tbd-factory-unlocked') === '1';
  var FACTORY_PIN = '0000';

  function _persistUnlock() {
    if (_unlocked) {
      sessionStorage.setItem('tbd-factory-unlocked', '1');
    } else {
      sessionStorage.removeItem('tbd-factory-unlocked');
    }
    // Dispatch event so footer lock icon can update
    window.dispatchEvent(new CustomEvent('tbd-factory-lock-changed', { detail: { unlocked: _unlocked } }));
  }

  /**
   * Show a Shoelace dialog asking for the factory PIN.
   * On success, sets unlocked=true and calls onSuccess().
   */
  function showPinDialog(onSuccess) {
    var old = document.getElementById('factory-pin-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'factory-pin-dialog';
    dialog.label = 'Factory Edit Mode';
    dialog.setAttribute('style', '--width:22rem;');

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0 0 0.75rem;">Enter the factory PIN to edit protected definitions.</p>'
      + '<sl-input id="factory-pin-input" type="password" placeholder="PIN" size="medium" '
      + 'style="width:100%;" autocomplete="off" autofocus></sl-input>'
      + '<p id="factory-pin-error" style="font-size:0.75rem;color:var(--sl-color-danger-600);margin:0.5rem 0 0;display:none;">Incorrect PIN</p>';

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var unlockBtn = document.createElement('sl-button');
    unlockBtn.setAttribute('slot', 'footer');
    unlockBtn.setAttribute('variant', 'warning');
    unlockBtn.setAttribute('style', 'margin-left: var(--sl-spacing-x-small);');
    unlockBtn.textContent = 'Unlock';

    function tryUnlock() {
      var input = document.getElementById('factory-pin-input');
      var errEl = document.getElementById('factory-pin-error');
      var val = input ? input.value.trim() : '';
      if (val === FACTORY_PIN) {
        _unlocked = true;
        _persistUnlock();
        dialog.hide();
        if (typeof onSuccess === 'function') onSuccess();
      } else {
        if (errEl) errEl.style.display = 'block';
        if (input) { input.value = ''; input.focus(); }
      }
    }

    unlockBtn.addEventListener('click', tryUnlock);

    // Allow Enter key to submit
    dialog.addEventListener('keydown', function(e) {
      if (e.key === 'Enter') { e.preventDefault(); tryUnlock(); }
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(unlockBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() {
      dialog.show();
      // Focus the input after dialog opens
      setTimeout(function() {
        var inp = document.getElementById('factory-pin-input');
        if (inp) inp.focus();
      }, 100);
    });
  }

  /**
   * Setup the footer lock button (present on both index.html and preset-macro-manager.html).
   * - Click when locked → show PIN dialog
   * - Click when unlocked → lock again
   */
  function setupFooterLock() {
    var btn = document.getElementById('factory-lock-btn');
    var icon = document.getElementById('factory-lock-icon');
    if (!btn || !icon) return;

    function updateIcon() {
      icon.setAttribute('name', _unlocked ? 'unlock' : 'lock');
      btn.classList.toggle('unlocked', _unlocked);
      btn.title = _unlocked ? 'Factory Edit Mode (unlocked) — click to lock' : 'Factory Edit Mode — click to unlock';
    }
    // Always defer initial icon update — even if sl-icon is already registered,
    // the specific element may not have completed its Lit upgrade cycle yet.
    // whenDefined resolves immediately (as microtask) if already defined,
    // then rAF ensures the element is fully upgraded before we set attributes.
    customElements.whenDefined('sl-icon').then(function() {
      requestAnimationFrame(updateIcon);
    });

    btn.addEventListener('click', function() {
      if (_unlocked) {
        _unlocked = false;
        _persistUnlock();
        updateIcon();
        if (window.TBD.shared && window.TBD.shared.toast) {
          window.TBD.shared.toast('Factory edit mode locked', 'neutral', 2000);
        }
      } else {
        showPinDialog(function() {
          updateIcon();
          if (window.TBD.shared && window.TBD.shared.toast) {
            window.TBD.shared.toast('Factory edit mode unlocked — factory files are now editable', 'warning', 3000);
          }
        });
      }
    });

    // Listen for lock/unlock events from other code paths
    window.addEventListener('tbd-factory-lock-changed', function() { updateIcon(); });
  }

  window.TBD = window.TBD || {};
  window.TBD.factory = {
    isFactoryDefinition: function(id) { return defSet[id] === true; },
    isFactoryPreset: function(id) { return presetSet[id] === true; },
    isUnlocked: function() { return _unlocked; },
    showPinDialog: showPinDialog,
    setupFooterLock: setupFooterLock,
    lock: function() { _unlocked = false; _persistUnlock(); },
    FACTORY_DEFINITIONS: FACTORY_DEFINITIONS,
    FACTORY_PRESETS: FACTORY_PRESETS,
  };
})();

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
    // Priority 0: Check the macro's explicit ui hint for a per-ui-type
    // override. Needed for per-machine ranges like MonoSynth's
    // "envattackfast" (0.5 ms..1 s) vs the generic "envattack"
    // (0.5 ms..5 s). Additive — old ui strings keep their current
    // range via the fallback lookups below.
    if (param && param.ui) {
      var UI_HINTS = {
        envattack:     { unit: 'ms', scale: 'log', physMin: 0.5, physMax: 5000, label: 'Attack' },
        envattackfast: { unit: 'ms', scale: 'log', physMin: 0.5, physMax: 1000, label: 'Attack' },
        envdecay:      { unit: 'ms', scale: 'log', physMin: 1,   physMax: 5000, label: 'Decay' },
      };
      if (UI_HINTS[param.ui]) {
        return Object.assign({}, UI_HINTS[param.ui], { label: paramName || UI_HINTS[param.ui].label });
      }
    }

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

// ── performer.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Unified View Controller
//
// Manages the center panel knob controls and the left sidebar
// preset browser. Uses shared renderKnobGroups() for consistent
// knob rendering across all modes.
//
// Data flow:
//   Track Selection → Machine → Macro Def → Knobs
//   Sound Presets → Load knob values
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    activeTrack: -1,
    activeMachine: '',
    activeMacroDef: null,
    activePreset: null,
    paramValues: [],
    presetSearchTerm: '',
    macroFilter: null,   // null = show all macros, string = filter by def id
    initialized: false,
  };

  // ─── Track Selection (driven by shared track tabs) ────────

  function onTrackSelected(idx, track) {
    state.activeTrack = idx;
    state.activePreset = null;
    state.macroFilter = null; // reset filter on track change

    var availMachines = S.getTrackMachines(track);

    // Check for boot default preset
    var bootPreset = null;
    var bootDef = null;
    if (S.data.trackDefaults && S.data.trackDefaults.tracks) {
      var tdEntry = S.data.trackDefaults.tracks.find(function(t) { return t.index === idx; });
      if (tdEntry && tdEntry.preset) {
        bootPreset = S.data.soundPresets.find(function(p) { return p.id === tdEntry.preset; });
        if (bootPreset) {
          bootDef = S.data.macroDefs.find(function(d) { return d.id === bootPreset.macro; });
        }
      }
    }

    if (bootPreset && bootDef) {
      // Auto-select the boot default preset
      state.activePreset = bootPreset;
      state.activeMacroDef = bootDef;
      state.activeMachine = bootDef.machine;
      state.macroFilter = bootPreset.macro;

      // Load param values from preset
      state.paramValues = [];
      if (bootPreset.values && bootPreset.values.length > 0) {
        state.paramValues = bootPreset.values.slice();
        for (var vi = 0; vi < state.paramValues.length; vi++) {
          if (state.paramValues[vi] === undefined || state.paramValues[vi] === null) {
            state.paramValues[vi] = 0;
          }
        }
      }
      // Fill missing params from def defaults
      if (bootDef.groups) {
        bootDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (state.paramValues[p.idx] === undefined) {
              state.paramValues[p.idx] = p.def || 0;
            }
          });
        });
      }
    } else {
      // Fallback: default to first available machine + allparams def
      var machineId = availMachines.length > 0 ? availMachines[0] : '';
      state.activeMachine = machineId;

      var matchingDefs = S.data.macroDefs.filter(function(d) {
        return d.machine === machineId;
      });
      var allParamsDef = matchingDefs.find(function(d) {
        return d.id.indexOf('allparams') !== -1;
      });
      var def = allParamsDef || matchingDefs[0] || null;
      state.activeMacroDef = def;

      state.paramValues = [];
      if (def && def.groups) {
        def.groups.forEach(function(group) {
          group.parameters.forEach(function(param) {
            state.paramValues[param.idx] = param.def || 0;
          });
        });
      }
    }

    renderMachineSelect(availMachines);
    renderKnobControls(track, state.activeMacroDef);
    renderPresetBrowser();

    // Notify designer of machine change
    if (window.TBD.designer && window.TBD.designer.onMachineChanged) {
      window.TBD.designer.onMachineChanged(state.activeMachine);
    }
  }

  // ─── Machine Select (rendered in track-info-bar) ─────────

  function renderMachineSelect(availMachines) {
    // Machine select is now rendered inside renderTrackInfoBar
    // Keep state updated for external access
    state._availMachines = availMachines;
  }

  // ─── Knob Controls Rendering ─────────────────────────────

  function renderTrackInfoBar(track, macroDef) {
    var trackBar = document.getElementById('track-info-bar');
    if (!trackBar || !track) return;

    var html = '<div class="track-info-header">';

    // Left: Badge + Name
    html += '<div class="track-info-left">';
    html += '<span class="track-badge">CH ' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-title">' + S.esc(track.name) + '</span>';
    html += '</div>';

    html += '<span class="track-info-separator"></span>';

    // Right: stacked rows
    html += '<div class="track-info-right">';

    // Row 1: MACHINE (+ MACRO in Presets mode)
    var activeTab = (S.getActiveTab && S.getActiveTab()) || 'presets';
    var matchingDefs = S.data.macroDefs.filter(function(d) {
      return d.machine === state.activeMachine;
    });

    html += '<div class="track-info-row">';
    html += '<span class="track-info-label">MACHINE:</span>';
    var availMachines = state._availMachines || S.getTrackMachines(track);
    if (availMachines.length > 1) {
      html += '<sl-select id="machine-select" size="small" value="' + S.esc(state.activeMachine || '') + '" style="min-width:140px;flex:1;max-width:200px;" hoist>';
      availMachines.forEach(function(machId) {
        var info = S.getMachineInfo(machId);
        var label = info ? info.name : machId;
        html += '<sl-option value="' + S.esc(machId) + '">' + S.esc(label) + '</sl-option>';
      });
      html += '</sl-select>';
    } else {
      var machInfo = S.getMachineInfo(state.activeMachine);
      var machName = machInfo ? machInfo.name : state.activeMachine;
      if (machName) {
        html += '<span class="track-machine-label">' + S.esc(machName) + '</span>';
      }
    }
    // In Presets mode, add MACRO dropdown in the same row as MACHINE
    if (activeTab !== 'macros' && matchingDefs.length > 0) {
      if (matchingDefs.length > 1) {
        var filterVal = state.macroFilter || '__all__';
        html += '<span class="track-info-label" style="margin-left:0.5rem;">MACRO:</span>';
        html += '<sl-select id="knobset-select" size="small" value="' + S.esc(filterVal) + '" style="min-width:140px;flex:1;max-width:200px;" hoist>';
        html += '<sl-option value="__all__">All Macros</sl-option>';
        matchingDefs.forEach(function(d) {
          html += '<sl-option value="' + S.esc(d.id) + '">' + S.esc(d.name || d.id) + '</sl-option>';
        });
        html += '</sl-select>';
      } else if (macroDef) {
        html += '<span class="track-info-label" style="margin-left:0.5rem;">MACRO:</span>';
        html += '<span class="track-knobset-label">' + S.esc(macroDef.name) + '</span>';
      }
    }
    html += '</div>'; // .track-info-row

    // Row 2 divider + content
    var hasRow2 = (activeTab === 'macros' && macroDef) || activeTab !== 'macros';
    if (hasRow2) {
      html += '<hr class="track-info-divider" />';
    }

    // Row 2: Context-dependent
    if (activeTab === 'macros') {
      var def = macroDef;
      if (def) {
        var D = window.TBD.designer;
        var isNew = D && D.state && !D.state.selectedDefId;
        html += '<div class="track-info-row" style="flex-wrap:nowrap;gap:0.35rem;">';
        html += '<span class="track-info-label">MACRO NAME:</span>';
        html += '<input class="track-inline-input def-name-input" value="' + S.esc(def.name) + '" placeholder="Definition name" style="flex:1 1 80px;min-width:60px;max-width:200px;" />';
        html += '<span class="track-info-label">ID:</span>';
        html += '<input class="track-inline-input def-id-input" value="' + S.esc(def.id) + '" placeholder="auto-id" style="min-width:0;max-width:12ch;" ' + (isNew ? '' : 'readonly') + ' />';
        var F = window.TBD.factory;
        var isFactoryDef = F && F.isFactoryDefinition(def.id);
        var isFactoryUnlocked = F && F.isUnlocked && F.isUnlocked();
        var volReadonly = isFactoryDef && !isFactoryUnlocked;
        html += '<span class="track-info-label" title="Volume multiplier — compensates for quiet/loud engines. 1.0 = no change.">VOL:</span>';
        html += '<input type="text" inputmode="decimal" class="track-inline-input def-volmult-input" value="' + (def.volmult != null ? def.volmult : 1.0) + '" style="width:5ch;min-width:4ch;max-width:6ch;text-align:center;padding:0.25rem 0.2rem;' + (volReadonly ? 'opacity:0.5;' : '') + '" title="Volume multiplier (0.1–4.0)"' + (volReadonly ? ' readonly' : '') + ' />';
        html += '<div class="track-def-actions" style="flex-shrink:0;">';
        html += '<button class="mapping-btn btn-save-def" title="Save this definition"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save</button>';
        html += '<button class="mapping-btn btn-export-def" title="Export as JSON"><sl-icon name="download" style="font-size:0.7rem;"></sl-icon> Export</button>';
        html += '<button class="mapping-btn btn-import-def" title="Import from JSON"><sl-icon name="upload" style="font-size:0.7rem;"></sl-icon> Import</button>';
        var jsonFolder = (isFactoryDef ? 'factory/macros' : 'macros');
        var jsonFile = jsonFolder + '/' + def.id + '.json';
        html += '<a class="mapping-btn btn-viewjson-def" href="/index.html?view=samples&file=' + encodeURIComponent(jsonFile) + '" title="View this macro\'s JSON in Data Manager" style="text-decoration:none;"><sl-icon name="filetype-json" style="font-size:0.7rem;"></sl-icon> View JSON</a>';
        if (!isNew) {
          html += '<button class="mapping-btn btn-delete-def" title="Delete this definition" style="border-color:var(--sl-color-danger-300);color:var(--sl-color-danger-600);"><sl-icon name="trash3" style="font-size:0.7rem;"></sl-icon> Delete</button>';
        }
        html += '</div>';
        html += '</div>';
      }
    } else {
      // Presets mode — row 2: Preset info + action buttons
      html += '<div class="track-info-row" style="flex-wrap:nowrap;gap:0.35rem;">';
      if (state.activePreset) {
        var F = window.TBD.factory;
        var isFactoryPreset = F && F.isFactoryPreset(state.activePreset.id);
        var isFactoryUnlocked = F && F.isUnlocked && F.isUnlocked();
        if (isFactoryPreset && !isFactoryUnlocked) {
          html += '<sl-icon name="lock" style="font-size:0.6rem;opacity:0.45;flex-shrink:0;" title="Factory preset — read-only"></sl-icon>';
        }
        html += '<span class="track-info-label">PRESET:</span>';
        html += '<span class="track-preset-name">' + S.esc(state.activePreset.name) + '</span>';
        html += '<span class="track-info-label" style="margin-left:0.25rem;">ID:</span>';
        html += '<span class="track-preset-id">' + S.esc(state.activePreset.id) + '</span>';
      } else {
        html += '<span class="track-info-label">PRESET:</span>';
        html += '<span class="track-preset-name" style="opacity:0.4;font-style:italic;">No preset loaded</span>';
      }
      // Preset action buttons
      html += '<div class="track-def-actions" style="flex-shrink:0;">';
      if (state.activePreset) {
        var Fsave = window.TBD.factory;
        var isFactoryP = Fsave && Fsave.isFactoryPreset(state.activePreset.id);
        var isUnlockedP = Fsave && Fsave.isUnlocked && Fsave.isUnlocked();
        var canOverwrite = !isFactoryP || isUnlockedP;
        if (canOverwrite) {
          html += '<button class="mapping-btn btn-save-preset" title="Overwrite current preset with current knob values"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save</button>';
        }
      }
      html += '<button class="mapping-btn btn-saveas-preset" title="Save current knob values as a new preset"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save As\u2026</button>';
      html += '<button class="mapping-btn btn-export-presets" title="Export all presets as JSON"><sl-icon name="download" style="font-size:0.7rem;"></sl-icon> Export</button>';
      html += '<button class="mapping-btn btn-import-presets" title="Import presets from JSON"><sl-icon name="upload" style="font-size:0.7rem;"></sl-icon> Import</button>';
      if (state.activePreset) {
        var FJ = window.TBD.factory;
        var jsonFolder = (FJ && FJ.isFactoryPreset(state.activePreset.id)) ? 'factory/presets' : 'presets';
        var jsonFile = jsonFolder + '/' + state.activePreset.id + '.json';
        html += '<a class="mapping-btn btn-viewjson-presets" href="/index.html?view=samples&file=' + encodeURIComponent(jsonFile) + '" title="View this preset\u2019s JSON in Data Manager" style="text-decoration:none;"><sl-icon name="filetype-json" style="font-size:0.7rem;"></sl-icon> JSON</a>';
      } else {
        html += '<a class="mapping-btn btn-viewjson-presets" href="/index.html?view=samples&browse=presets" title="Browse preset JSON files in Data Manager" style="text-decoration:none;"><sl-icon name="filetype-json" style="font-size:0.7rem;"></sl-icon> JSON</a>';
      }
      html += '</div>';
      html += '</div>';
    }

    html += '</div>'; // .track-info-right
    html += '</div>'; // .track-info-header
    trackBar.innerHTML = html;
    setupMachineSelectEvents();
  }

  function setupMachineSelectEvents() {
    var machineSelect = document.getElementById('machine-select');
    if (machineSelect) {
      machineSelect.addEventListener('sl-change', function() {
        onMachineChange(machineSelect.value);
        if (window.TBD.designer && window.TBD.designer.onMachineChanged) {
          window.TBD.designer.onMachineChanged(machineSelect.value);
        }
      });
    }

    // Knob set dropdown (Presets mode — acts as macro filter)
    var knobsetSelect = document.getElementById('knobset-select');
    if (knobsetSelect) {
      knobsetSelect.addEventListener('sl-change', function() {
        var defId = knobsetSelect.value;
        if (defId === '__all__') {
          // "All Macros" — clear filter, keep current activeMacroDef
          state.macroFilter = null;
          state.activePreset = null;
          var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
          if (track) renderTrackInfoBar(track, state.activeMacroDef);
        } else {
          var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
          if (def) {
            state.macroFilter = defId;
            state.activeMacroDef = def;
            state.activePreset = null;
            state.paramValues = [];
            if (def.groups) {
              def.groups.forEach(function(group) {
                group.parameters.forEach(function(param) {
                  state.paramValues[param.idx] = param.def || 0;
                });
              });
            }
            var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
            if (track) {
              renderKnobControls(track, def);
            }
          }
        }
        renderPresetBrowser();
      });
    }

    // Preset action buttons in header bar (Presets mode)
    var savePresetBtn = document.querySelector('#track-info-bar .btn-save-preset');
    if (savePresetBtn) {
      savePresetBtn.addEventListener('click', function() {
        savePresetOverwrite();
      });
    }
    var saveAsPresetBtn = document.querySelector('#track-info-bar .btn-saveas-preset');
    if (saveAsPresetBtn) {
      saveAsPresetBtn.addEventListener('click', function() {
        if (state.activeTrack < 0 || !state.activeMacroDef) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        savePresetDialog();
      });
    }
    var exportPresetsBtn = document.querySelector('#track-info-bar .btn-export-presets');
    if (exportPresetsBtn) {
      exportPresetsBtn.addEventListener('click', function() {
        exportAllPresets();
      });
    }
    var importPresetsBtn = document.querySelector('#track-info-bar .btn-import-presets');
    if (importPresetsBtn) {
      importPresetsBtn.addEventListener('click', function() {
        importPresetFile();
      });
    }

    // Def header inputs (Macros mode — NAME/ID/Save/Export/Import in track bar)
    setupDefHeaderInBarEvents();
  }

  function setupDefHeaderInBarEvents() {
    var D = window.TBD.designer;
    if (!D || !D.state) return;

    var nameInput = document.querySelector('#track-info-bar .def-name-input');
    var idInput = document.querySelector('#track-info-bar .def-id-input');

    if (nameInput) {
      nameInput.addEventListener('input', function() {
        if (D.state.editDef) {
          D.state.editDef.name = nameInput.value;
          D.state.dirty = true;
          // Auto-generate ID for new definitions (only if user hasn't manually edited ID)
          if (!D.state.selectedDefId && idInput && !idInput.dataset.userEdited) {
            var machinePrefix = D.state.editDef.machine ? (D.state.editDef.machine.substring(0, 2) + '-') : '';
            var slug = nameInput.value.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
            D.state.editDef.id = machinePrefix + slug;
            idInput.value = D.state.editDef.id;
          }
        }
      });
      nameInput.addEventListener('change', function() {
        if (D.state.editDef) {
          D.state.editDef.name = nameInput.value;
          D.state.dirty = true;
        }
      });
    }

    if (idInput) {
      idInput.addEventListener('input', function() {
        idInput.dataset.userEdited = 'true';
      });
      idInput.addEventListener('change', function() {
        if (D.state.editDef) {
          D.state.editDef.id = idInput.value;
          D.state.dirty = true;
        }
      });
    }

    var volmultInput = document.querySelector('#track-info-bar .def-volmult-input');
    if (volmultInput) {
      volmultInput.addEventListener('change', function() {
        if (D.state.editDef) {
          var v = parseFloat(volmultInput.value);
          if (isNaN(v) || v < 0.1) v = 0.1;
          if (v > 4.0) v = 4.0;
          v = Math.round(v * 10) / 10; // round to 1 decimal
          volmultInput.value = v;
          D.state.editDef.volmult = v;
          D.state.dirty = true;
        }
      });
    }

    var saveBtn = document.querySelector('#track-info-bar .btn-save-def');
    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        if (D.saveDefinition) D.saveDefinition();
      });
    }
    var exportBtn = document.querySelector('#track-info-bar .btn-export-def');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        if (D.exportDefinition) D.exportDefinition();
      });
    }
    var importBtn = document.querySelector('#track-info-bar .btn-import-def');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        if (D.importDefinitionFile) D.importDefinitionFile();
      });
    }
    var deleteDefBtn = document.querySelector('#track-info-bar .btn-delete-def');
    if (deleteDefBtn) {
      deleteDefBtn.addEventListener('click', function() {
        if (D.state && D.state.selectedDefId) {
          if (D.deleteDefinition) D.deleteDefinition(D.state.selectedDefId);
        }
      });
    }
  }

  function renderKnobControls(track, macroDef) {
    var container = document.getElementById('knob-controls');
    if (!container) return;

    // Render the track info bar (separated from knob content)
    renderTrackInfoBar(track, macroDef);

    if (!macroDef) {
      container.innerHTML =
        '<div class="empty-state" id="knob-empty">' +
        '<sl-icon name="sliders"></sl-icon>' +
        '<h3>No macro definition found</h3>' +
        '<p>No macro definition available for machine "' + S.esc(state.activeMachine) + '"</p>' +
        '</div>';
      return;
    }

    // Render knob groups using the shared renderer
    container.innerHTML = S.renderKnobGroups(macroDef, state.paramValues);
    setupKnobEvents(container);
    setupGroupCollapseEvents(container);
  }

  function setupGroupCollapseEvents(container) {
    container.querySelectorAll('.macro-group-header').forEach(function(header) {
      header.addEventListener('click', function() {
        header.parentElement.classList.toggle('collapsed');
      });
    });
  }

  // ─── Knob Drag Interaction ───────────────────────────────

  function setupKnobEvents(container) {
    container.querySelectorAll('.macro-knob').forEach(function(knob) {
      var cell = knob.closest('.macro-knob-cell');
      var valueEl = cell.querySelector('.macro-knob-value');
      var min = parseInt(knob.getAttribute('data-min'), 10) || 0;
      var max = parseInt(knob.getAttribute('data-max'), 10) || 127;
      var paramIdx = parseInt(knob.getAttribute('data-idx'), 10);
      var startY = 0;
      var startVal = 0;

      function onPointerDown(e) {
        e.preventDefault();
        knob.classList.add('dragging');
        startY = e.clientY;
        startVal = parseInt(knob.getAttribute('data-value'), 10) || 0;
        document.addEventListener('pointermove', onPointerMove);
        document.addEventListener('pointerup', onPointerUp);
      }

      function onPointerMove(e) {
        var dy = startY - e.clientY;
        var range = max - min;
        var sensitivity = range / 200;
        var newVal = Math.round(startVal + dy * sensitivity);
        newVal = Math.max(min, Math.min(max, newVal));

        knob.setAttribute('data-value', newVal);
        valueEl.textContent = newVal;
        state.paramValues[paramIdx] = newVal;

        var knobColor = knob.getAttribute('data-color') || 'normal';
        knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: knobColor, size: 64 });

        // Update target panel values in real-time
        if (state.activeMacroDef) {
          var panel = cell.querySelector('.knob-target-panel');
          if (panel) {
            var outputs = S.computeMappingOutputs(state.activeMacroDef, paramIdx, newVal);
            var targetDH = window.TBD && window.TBD.displayHints;
            outputs.forEach(function(o) {
              var row = panel.querySelector('.knob-target-row[data-ctrl="' + o.ctrl + '"]');
              if (!row) return;
              var valEl = row.querySelector('.knob-target-val');
              var dotEl = row.querySelector('.knob-target-dot');
              if (valEl) {
                var fmt = String(o.value);
                if (targetDH && state.activeMacroDef.machine) {
                  var pid = state.activeMacroDef.machine + '_' + o.name.replace(/[- ]/g, '_');
                  var hint = targetDH.resolveHint(pid, o.name);
                  if (hint) {
                    var physVal = targetDH.rawToDisplay(o.value, 0, o.max || 127, hint);
                    fmt = targetDH.formatDisplayValue(physVal, hint);
                  }
                }
                valEl.textContent = fmt;
              }
              if (dotEl) dotEl.style.left = (o.value / (o.max || 127) * 100) + '%';
            });
          }
        }
      }

      function onPointerUp() {
        knob.classList.remove('dragging');
        document.removeEventListener('pointermove', onPointerMove);
        document.removeEventListener('pointerup', onPointerUp);
        var value = parseInt(knob.getAttribute('data-value'), 10);
        state.paramValues[paramIdx] = value;
        sendParameterUpdate();
      }

      knob.addEventListener('pointerdown', onPointerDown);
    });
  }

  // ─── API: Send Updates ────────────────────────────────────

  function sendTrackUpdate(body) {
    S.apiPostJSON('/macros?action=update_track', body).then(function() {
      console.log('[Performer] Track update sent:', body);
    }).catch(function(err) {
      console.error('[Performer] Track update failed:', err);
    });
  }

  function sendParameterUpdate() {
    if (state.activeTrack < 0 || !state.activeMacroDef) return;
    var body = {
      track: state.activeTrack,
      machine: state.activeMachine,
      macro: state.activeMacroDef.id,
      parameters: state.paramValues.slice(),
    };
    S.apiPostJSON('/macros?action=update_track', body).then(function() {
      console.log('[Performer] Parameters sent for track', state.activeTrack);
    }).catch(function(err) {
      console.error('[Performer] Parameter send failed:', err);
    });
  }

  // ─── Preset Browser ─────────────────────────────────────

  function renderPresetBrowser() {
    var container = document.getElementById('preset-list');
    if (!container) return;

    // Collect ALL presets for this machine (for total count)
    var machineDefIds = {};
    S.data.macroDefs.forEach(function(d) {
      if (d.machine === state.activeMachine) {
        machineDefIds[d.id] = true;
      }
    });
    var allMachinePresets = S.data.soundPresets.filter(function(p) {
      return machineDefIds[p.macro] || false;
    });
    var totalCount = allMachinePresets.length;

    // Apply macro filter
    var presets;
    if (state.macroFilter) {
      presets = allMachinePresets.filter(function(p) {
        return p.macro === state.macroFilter;
      });
    } else {
      presets = allMachinePresets;
    }

    var term = state.presetSearchTerm.toLowerCase();
    if (term) {
      presets = presets.filter(function(p) {
        return (p.name || '').toLowerCase().indexOf(term) !== -1 ||
               (p.group || '').toLowerCase().indexOf(term) !== -1;
      });
    }

    var filteredCount = presets.length;

    // Build HTML — filter chip + count + list
    var html = '';

    // Active filter chip (dismissible)
    if (state.macroFilter) {
      var filterDef = S.data.macroDefs.find(function(d) { return d.id === state.macroFilter; });
      var filterLabel = filterDef ? (filterDef.name || filterDef.id) : state.macroFilter;
      html += '<div class="preset-filter-bar">';
      html += '<button class="preset-filter-chip" id="clear-macro-filter" title="Show all macros">';
      html += '<span class="preset-filter-chip-label">' + S.esc(filterLabel) + '</span>';
      html += '<sl-icon name="x-lg" style="font-size:0.6rem;"></sl-icon>';
      html += '</button>';
      html += '<span class="preset-count">' + filteredCount + ' of ' + totalCount + '</span>';
      html += '</div>';
    } else if (totalCount > 0) {
      html += '<div class="preset-filter-bar">';
      html += '<span class="preset-count">' + totalCount + ' preset' + (totalCount !== 1 ? 's' : '') + '</span>';
      html += '</div>';
    }

    // Group by category
    var groups = {};
    presets.forEach(function(p) {
      var g = p.group || 'Uncategorized';
      if (!groups[g]) groups[g] = [];
      groups[g].push(p);
    });

    var F = window.TBD.factory;

    Object.keys(groups).sort().forEach(function(groupName) {
      html += '<div class="preset-category">' + S.esc(groupName) + '</div>';
      groups[groupName].forEach(function(p) {
        var isActive = state.activePreset && state.activePreset.id === p.id;
        var isFactory = F && F.isFactoryPreset(p.id);
        html += '<div class="preset-item' + (isActive ? ' active' : '') + '" data-preset-id="' + S.esc(p.id) + '">';
        if (isFactory) {
          html += '<sl-icon name="lock" style="font-size:0.6rem;opacity:0.4;flex-shrink:0;margin-right:0.2rem;" title="Factory preset — use Save As to create a copy"></sl-icon>';
        }
        html += '<span class="preset-item-name" title="' + S.esc(p.name) + '">' + S.esc(p.name) + '</span>';
        html += '<span class="preset-item-machine">' + S.esc(p.macro) + '</span>';
        if (!isFactory || (F && F.isUnlocked && F.isUnlocked())) {
          html += '<button class="preset-item-delete" data-delete-preset-id="' + S.esc(p.id) + '" title="Delete preset">';
          html += '<sl-icon name="trash3"></sl-icon>';
          html += '</button>';
        }
        html += '</div>';
      });
    });

    if (filteredCount === 0) {
      html += '<div class="empty-state" style="padding:1.5rem;">';
      if (state.macroFilter) {
        html += '<p style="font-size:0.78rem;">No presets for this macro</p>';
        html += '<p style="font-size:0.7rem;color:var(--sl-color-neutral-400);">Clear the filter to see all presets</p>';
      } else {
        html += '<p style="font-size:0.78rem;">No presets for ' + S.esc(state.activeMachine || 'this track') + '</p>';
      }
      html += '</div>';
    }

    container.innerHTML = html;

    // Wire up the filter chip dismiss button
    var clearBtn = document.getElementById('clear-macro-filter');
    if (clearBtn) {
      clearBtn.addEventListener('click', function() {
        state.macroFilter = null;
        // Re-render track info bar to sync dropdown to "All Macros"
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) {
          renderTrackInfoBar(track, state.activeMacroDef);
        }
        renderPresetBrowser();
      });
    }
  }

  function setupPresetBrowserEvents() {

    var list = document.getElementById('preset-list');
    if (list) {
      list.addEventListener('click', function(e) {
        // Handle delete button click
        var deleteBtn = e.target.closest('.preset-item-delete');
        if (deleteBtn) {
          e.stopPropagation();
          var presetId = deleteBtn.getAttribute('data-delete-preset-id');
          if (presetId) deletePreset(presetId);
          return;
        }
        var item = e.target.closest('.preset-item');
        if (!item) return;
        var presetId = item.getAttribute('data-preset-id');
        loadPreset(presetId);
      });
    }
  }

  function loadPreset(presetId) {
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    if (!preset) return;

    state.activePreset = preset;

    var def = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
    if (def) {
      state.activeMacroDef = def;
      state.activeMachine = def.machine;
    }

    // Sync macro filter to reflect the preset's macro
    state.macroFilter = preset.macro;

    if (preset.values && preset.values.length > 0) {
      state.paramValues = preset.values.slice();
      // Ensure all values are defined (no nulls/undefineds from sparse arrays)
      for (var vi = 0; vi < state.paramValues.length; vi++) {
        if (state.paramValues[vi] === undefined || state.paramValues[vi] === null) {
          state.paramValues[vi] = 0;
        }
      }
    }

    // If definition has more params than the preset, fill missing values with defaults
    if (def && def.groups) {
      def.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) {
          if (state.paramValues[p.idx] === undefined) {
            state.paramValues[p.idx] = p.def || 0;
          }
        });
      });
    }

    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      // Re-render track info bar so MACRO dropdown reflects the preset's macro
      renderTrackInfoBar(track, state.activeMacroDef);
      renderKnobControls(track, state.activeMacroDef);
    }

    // Re-render sidebar (filter chip + filtered list with active highlight)
    renderPresetBrowser();

    sendTrackUpdate({
      track: state.activeTrack,
      machine: state.activeMachine,
      macro: preset.macro,
      parameters: state.paramValues.slice(),
    });

    S.toast('Loaded: ' + preset.name, 'success', 2000);
  }

  // ─── Quick Actions (sidebar buttons) ─────────────────────

  function setupQuickActions() {
    var newPresetBtn = document.getElementById('qa-new-preset');
    if (newPresetBtn) {
      newPresetBtn.addEventListener('click', function() {
        if (state.activeTrack < 0) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        // Check if any macros are available for the current machine
        var availableMacros = S.data.macroDefs.filter(function(d) {
          return d.machine === state.activeMachine;
        });
        if (availableMacros.length === 0) {
          S.toast('No macros available for this machine — create one first in the Macros tab', 'warning', 3000);
          return;
        }
        savePresetDialog();
      });
    }

  }

  // ─── Save Preset Overwrite (existing preset) ──────────────

  function savePresetOverwrite() {
    if (!state.activePreset || !state.activeMacroDef) {
      S.toast('No preset loaded to save', 'warning', 2000);
      return;
    }

    // Double-check: factory presets require unlocked factory edit mode
    var F = window.TBD.factory;
    var isFactory = F && F.isFactoryPreset(state.activePreset.id);
    if (isFactory && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory presets are read-only — use Save As to create a copy', 'warning', 3000);
      return;
    }

    // Gather current knob values
    var paramCount = 0;
    if (state.activeMacroDef.groups) {
      state.activeMacroDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) {
          if (p.idx >= paramCount) paramCount = p.idx + 1;
        });
      });
    }
    var values = [];
    for (var vi = 0; vi < paramCount; vi++) {
      var raw = state.paramValues[vi];
      values[vi] = (raw !== undefined && raw !== null) ? Math.round(raw) : 0;
    }

    var preset = {
      id: state.activePreset.id,
      name: state.activePreset.name,
      group: state.activePreset.group || 'User',
      macro: state.activeMacroDef.id,
      values: values,
    };

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = (isFactory ? 'factory/presets/' : 'presets/') + preset.id + '.json';

    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderPresetBrowser();
    }).catch(function(err) {
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── Save Preset Dialog (Shoelace) ────────────────────────

  function savePresetDialog() {
    // Remove any old dialog
    var old = document.getElementById('save-preset-dialog');
    if (old) old.remove();

    var F = window.TBD.factory;
    var isFromFactory = state.activePreset && F && F.isFactoryPreset(state.activePreset.id);

    var defaultName = state.activePreset ? state.activePreset.name : (state.activeMacroDef ? state.activeMacroDef.name : '');
    if (isFromFactory) defaultName = defaultName + ' (copy)';
    var defaultGroup = state.activePreset ? (state.activePreset.group || '') : '';
    if (isFromFactory && defaultGroup) defaultGroup = 'User';
    if (!defaultGroup) defaultGroup = 'User';

    var trackName = '';
    var trackBadge = '';
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      trackName = track.name;
      trackBadge = 'CH ' + String(track.index + 1).padStart(2, '0');
    }

    // Available machines for this track
    var availMachines = track ? S.getTrackMachines(track) : [];
    var currentMachine = state.activeMachine || (availMachines.length > 0 ? availMachines[0] : '');
    var currentMacroId = state.activeMacroDef ? state.activeMacroDef.id : '';

    // Dialog state — track selections via sl-change events
    var selectedMachine = currentMachine;
    var selectedMacroId = currentMacroId;

    function getMacrosForMachine(machId) {
      return S.data.macroDefs.filter(function(d) { return d.machine === machId; });
    }

    function buildMacroOptions(machId) {
      var macros = getMacrosForMachine(machId);
      var h = '';
      macros.forEach(function(m) {
        h += '<sl-option value="' + S.esc(m.id) + '">' + S.esc(m.name || m.id) + '</sl-option>';
      });
      return h;
    }

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'save-preset-dialog';
    dialog.label = 'New Sound Preset';
    dialog.setAttribute('style', '--width:28rem;');

    var html = '';
    // Context header
    html += '<div class="save-preset-context">';
    html += '<span class="track-badge" style="font-size:0.72rem;">' + S.esc(trackBadge) + '</span> ';
    html += '<strong>' + S.esc(trackName) + '</strong>';
    html += '</div>';

    html += '<div style="display:flex;flex-direction:column;gap:0.75rem;margin-top:0.75rem;">';
    // Machine dropdown (first)
    if (availMachines.length > 1) {
      html += '<sl-select id="save-preset-machine" label="Machine" value="' + S.esc(currentMachine) + '" hoist>';
      availMachines.forEach(function(machId) {
        var info = S.getMachineInfo(machId);
        var label = info ? info.name : machId;
        html += '<sl-option value="' + S.esc(machId) + '">' + S.esc(label) + '</sl-option>';
      });
      html += '</sl-select>';
    } else {
      var mInfo = S.getMachineInfo(currentMachine);
      var mName = mInfo ? mInfo.name : currentMachine;
      html += '<div style="font-size:0.78rem;color:var(--sl-color-neutral-500);"><strong>Machine:</strong> ' + S.esc(mName) + '</div>';
    }
    // Macro dropdown (second)
    html += '<sl-select id="save-preset-macro" label="Macro" value="' + S.esc(currentMacroId) + '" hoist>';
    html += buildMacroOptions(currentMachine);
    html += '</sl-select>';
    // Preset name
    html += '<sl-input id="save-preset-name" label="Preset Name" value="' + S.esc(defaultName) + '" placeholder="e.g. Fat Punch" required></sl-input>';
    // Category
    html += '<sl-input id="save-preset-group" label="Category / Group" value="' + S.esc(defaultGroup) + '" placeholder="e.g. User" help-text="Presets are grouped by this label in the sidebar"></sl-input>';
    html += '</div>';
    html += '<div style="margin-top:1rem;font-size:0.72rem;color:var(--sl-color-neutral-500);">';
    html += '<sl-icon name="info-circle" style="font-size:0.7rem;"></sl-icon> ';
    html += 'Saves the current knob values as a new sound preset for the selected macro.';
    html += '</div>';

    dialog.innerHTML = html;

    // Wire sl-change events to track selections reliably
    var machineSelectEl = dialog.querySelector('#save-preset-machine');
    if (machineSelectEl) {
      machineSelectEl.addEventListener('sl-change', function() {
        selectedMachine = machineSelectEl.value;
        // Rebuild Macro options for the new machine
        var macros = getMacrosForMachine(selectedMachine);
        var oldMacro = dialog.querySelector('#save-preset-macro');
        if (oldMacro) {
          var newMacro = document.createElement('sl-select');
          newMacro.id = 'save-preset-macro';
          newMacro.label = 'Macro';
          newMacro.setAttribute('hoist', '');
          newMacro.innerHTML = buildMacroOptions(selectedMachine);
          selectedMacroId = macros.length > 0 ? macros[0].id : '';
          newMacro.value = selectedMacroId;
          oldMacro.replaceWith(newMacro);
          newMacro.addEventListener('sl-change', function() {
            selectedMacroId = newMacro.value;
          });
        }
      });
    }
    var macroSelectEl = dialog.querySelector('#save-preset-macro');
    if (macroSelectEl) {
      macroSelectEl.addEventListener('sl-change', function() {
        selectedMacroId = macroSelectEl.value;
      });
    }

    // Footer buttons
    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var saveBtn = document.createElement('sl-button');
    saveBtn.setAttribute('slot', 'footer');
    saveBtn.setAttribute('variant', 'primary');
    saveBtn.innerHTML = '<sl-icon name="floppy" slot="prefix"></sl-icon> Save Preset';

    saveBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#save-preset-name');
      var groupInput = dialog.querySelector('#save-preset-group');
      var name = (nameInput.value || '').trim();
      var group = (groupInput.value || '').trim() || 'User';

      if (!name) {
        nameInput.setAttribute('help-text', 'Please enter a name');
        nameInput.focus();
        return;
      }

      if (!selectedMacroId) {
        S.toast('Please select a macro', 'warning', 2000);
        return;
      }

      // Find the selected macro definition
      var selectedMacroDef = S.data.macroDefs.find(function(d) { return d.id === selectedMacroId; });
      if (!selectedMacroDef) {
        S.toast('Selected macro not found', 'danger', 2000);
        return;
      }

      var id = name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');

      // Prevent overwriting factory presets (unless Factory Edit Mode unlocked)
      var Fcheck = window.TBD.factory;
      var isFactoryId = Fcheck && Fcheck.isFactoryPreset(id);
      if (isFactoryId && !(Fcheck.isUnlocked && Fcheck.isUnlocked())) {
        nameInput.setAttribute('help-text', 'This name matches a factory preset \u2014 unlock Factory Edit Mode or choose a different name');
        nameInput.focus();
        return;
      }

      // Produce a dense values array trimmed to the definition's parameter count
      var paramCount = 0;
      if (selectedMacroDef && selectedMacroDef.groups) {
        selectedMacroDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx >= paramCount) paramCount = p.idx + 1;
          });
        });
      }
      var values = [];
      for (var vi = 0; vi < paramCount; vi++) {
        var raw = state.paramValues[vi];
        values[vi] = (raw !== undefined && raw !== null) ? Math.round(raw) : 0;
      }
      var preset = {
        id: id,
        name: name,
        group: group,
        macro: selectedMacroId,
        values: values,
      };

      saveBtn.setAttribute('loading', '');
      var jsonStr = JSON.stringify(preset, null, 2);
      var filePath = (isFactoryId ? 'factory/presets/' : 'presets/') + id + '.json';

      fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: jsonStr,
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        dialog.hide();
        S.toast('Saved preset: ' + name, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderPresetBrowser();
      }).catch(function(err) {
        saveBtn.removeAttribute('loading');
        S.toast('Save failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(saveBtn);
    document.body.appendChild(dialog);

    // Prevent hoisted sl-select overlay clicks from closing the dialog
    dialog.addEventListener('sl-request-close', function(e) {
      if (e.detail.source === 'overlay') e.preventDefault();
    });

    // Guard: only remove on dialog's own hide, not bubbled sl-after-hide from child sl-selects
    dialog.addEventListener('sl-after-hide', function(e) {
      if (e.target !== dialog) return;
      dialog.remove();
    });

    // Show the dialog
    requestAnimationFrame(function() {
      dialog.show();
    });
  }

  // ─── Delete Preset ────────────────────────────────────────

  function deletePreset(presetId) {
    var F = window.TBD.factory;
    if (F && F.isFactoryPreset(presetId) && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory presets cannot be deleted \u2014 unlock Factory Edit Mode first', 'warning', 3000);
      return;
    }
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    var displayName = preset ? preset.name : presetId;

    var old = document.getElementById('delete-preset-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'delete-preset-dialog';
    dialog.label = 'Delete Sound Preset';
    dialog.setAttribute('style', '--width:24rem;');

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0;">Are you sure you want to delete <strong>' + S.esc(displayName) + '</strong>?</p>'
      + '<p style="font-size:0.75rem;color:var(--sl-color-neutral-500);margin:0.5rem 0 0;">This action cannot be undone.</p>';

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var deleteBtn = document.createElement('sl-button');
    deleteBtn.setAttribute('slot', 'footer');
    deleteBtn.setAttribute('variant', 'danger');
    deleteBtn.innerHTML = '<sl-icon name="trash3" slot="prefix"></sl-icon> Delete';
    deleteBtn.addEventListener('click', function() {
      deleteBtn.setAttribute('loading', '');
      var Fdel = window.TBD.factory;
      var filePath = (Fdel && Fdel.isFactoryPreset(presetId) ? 'factory/presets/' : 'presets/') + presetId + '.json';
      S.apiPostJSON('/storage?action=manage', { action: 'deleteconfig', path: filePath })
      .then(function() {
        dialog.hide();
        // If the deleted preset was active, clear it
        if (state.activePreset && state.activePreset.id === presetId) {
          state.activePreset = null;
        }
        S.toast('Deleted preset: ' + displayName, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderPresetBrowser();
        // Also refresh designer if it's active
        if (window.TBD.designer && window.TBD.designer.reload) {
          window.TBD.designer.reload();
        }
      }).catch(function(err) {
        deleteBtn.removeAttribute('loading');
        S.toast('Delete failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(deleteBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  // ─── Export / Import (for presets mode) ───────────────────

  function exportAllPresets() {
    // Patch the active preset's values with the current live knob state
    // so the export reflects what the user actually hears right now.
    var presets = S.data.soundPresets.map(function(p) {
      if (state.activePreset && p.id === state.activePreset.id && state.paramValues.length > 0) {
        var patched = {};
        for (var k in p) { if (p.hasOwnProperty(k)) patched[k] = p[k]; }
        var paramCount = 0;
        if (state.activeMacroDef && state.activeMacroDef.groups) {
          state.activeMacroDef.groups.forEach(function(g) {
            (g.parameters || []).forEach(function(pm) {
              if (pm.idx >= paramCount) paramCount = pm.idx + 1;
            });
          });
        }
        var vals = [];
        for (var i = 0; i < paramCount; i++) {
          var raw = state.paramValues[i];
          vals[i] = (raw !== undefined && raw !== null) ? Math.round(raw) : 0;
        }
        patched.values = vals;
        return patched;
      }
      return p;
    });
    var data = {
      macroDefs: S.data.macroDefs,
      soundPresets: presets,
    };
    var blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = 'tbd16-presets-export.json';
    a.click();
    URL.revokeObjectURL(a.href);
    S.toast('Exported all presets', 'success', 2000);
  }

  function importPresetFile() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', function() {
      if (!input.files.length) return;
      var reader = new FileReader();
      reader.onload = function() {
        try {
          var data = JSON.parse(reader.result);
          if (data.id && data.macro) {
            importSinglePreset(data);
          } else if (data.macroDefs || data.soundPresets) {
            importBulk(data);
          } else {
            S.toast('Unrecognized JSON format', 'warning', 3000);
          }
        } catch (err) {
          S.toast('Invalid JSON: ' + err.message, 'danger', 3000);
        }
      };
      reader.readAsText(input.files[0]);
    });
    input.click();
  }

  /**
   * Import a bulk export file containing macroDefs and/or soundPresets.
   * Each item is uploaded to the device sequentially.
   */
  function importBulk(data) {
    var defs = Array.isArray(data.macroDefs) ? data.macroDefs : [];
    var presets = Array.isArray(data.soundPresets) ? data.soundPresets : [];
    var total = defs.length + presets.length;
    if (total === 0) {
      S.toast('Nothing to import', 'warning', 2000);
      return;
    }
    if (!confirm('Import ' + defs.length + ' macro definitions and ' + presets.length + ' sound presets? Existing files with the same IDs will be overwritten.')) {
      return;
    }
    S.showLoading('Importing 0/' + total + '\u2026');
    var done = 0;
    var errors = 0;

    function uploadFile(path, obj) {
      return fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(path), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(obj, null, 2),
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        done++;
        S.showLoading('Importing ' + done + '/' + total + '\u2026');
      }).catch(function() {
        errors++;
        done++;
        S.showLoading('Importing ' + done + '/' + total + '\u2026');
      });
    }

    // Chain uploads sequentially to avoid overwhelming the device
    var chain = Promise.resolve();
    defs.forEach(function(d) {
      if (!d.id) return;
      chain = chain.then(function() {
        return uploadFile('macros/' + d.id + '.json', d);
      });
    });
    presets.forEach(function(p) {
      if (!p.id) return;
      chain = chain.then(function() {
        return uploadFile('presets/' + p.id + '.json', p);
      });
    });
    chain.then(function() {
      S.hideLoading();
      if (errors > 0) {
        S.toast('Imported with ' + errors + ' error(s)', 'warning', 3000);
      } else {
        S.toast('Imported ' + total + ' items', 'success', 2000);
      }
      return S.reloadFirmwareMacros().then(function() {
        return S.reloadMacroData();
      });
    }).then(function() {
      renderPresetBrowser();
    });
  }

  function importSinglePreset(preset) {
    if (!preset.id || !preset.macro || !Array.isArray(preset.values)) {
      S.toast('Invalid preset: missing id, macro, or values', 'danger', 3000);
      return;
    }
    // Verify the referenced macro definition exists
    var macroDef = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
    if (!macroDef) {
      S.toast('Macro definition "' + preset.macro + '" not found on device. Import the macro first.', 'warning', 4000);
      return;
    }
    // Count expected params and warn on mismatch
    var expectedCount = 0;
    (macroDef.groups || []).forEach(function(g) { expectedCount += (g.parameters || []).length; });
    if (preset.values.length !== expectedCount) {
      if (!confirm('Preset has ' + preset.values.length + ' values but macro "' + preset.macro + '" has ' + expectedCount + ' parameters. Import anyway?')) return;
    }
    var Fimp = window.TBD.factory;
    var isFactoryId = Fimp && Fimp.isFactoryPreset && Fimp.isFactoryPreset(preset.id);
    var filePath = (isFactoryId && Fimp.isUnlocked && Fimp.isUnlocked() ? 'factory/presets/' : 'presets/') + preset.id + '.json';
    var jsonStr = JSON.stringify(preset, null, 2);

    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Imported: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderPresetBrowser();
    }).catch(function(err) {
      S.toast('Import failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── Machine Change (toolbar) ────────────────────────────

  function onMachineChange(newMachine) {
    if (newMachine === state.activeMachine) return;
    state.activeMachine = newMachine;
    state.macroFilter = null; // reset filter on machine change

    var matchingDefs = S.data.macroDefs.filter(function(d) {
      return d.machine === newMachine;
    });
    var allParamsDef = matchingDefs.find(function(d) {
      return d.id.indexOf('allparams') !== -1;
    });
    var def = allParamsDef || matchingDefs[0] || null;
    state.activeMacroDef = def;

    state.paramValues = [];
    if (def && def.groups) {
      def.groups.forEach(function(group) {
        group.parameters.forEach(function(param) {
          state.paramValues[param.idx] = param.def || 0;
        });
      });
    }

    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    renderKnobControls(track, def);
    renderPresetBrowser();
    sendTrackUpdate({ track: state.activeTrack, machine: newMachine });
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    S.onTrackChange(function(idx, track) {
      onTrackSelected(idx, track);
    });

    setupPresetBrowserEvents();
    setupQuickActions();

    // Re-render when factory lock state changes (buttons, delete visibility)
    window.addEventListener('tbd-factory-lock-changed', function() {
      var track = S.data.tracks ? S.data.tracks.find(function(t) { return t.index === state.activeTrack; }) : null;
      if (track) renderTrackInfoBar(track, state.activeMacroDef);
      renderPresetBrowser();
    });

    if (S.data.loaded && S.data.tracks.length > 0) {
      S.selectTrack(S.data.tracks[0].index);
    }

    state.initialized = true;
  }

  // ─── Set Macro Def (called by designer in Macros mode) ────

  function setMacroDef(def) {
    state.activeMacroDef = def;
    if (def) {
      state.activeMachine = def.machine;
    }
    state.paramValues = [];
    if (def && def.groups) {
      def.groups.forEach(function(group) {
        group.parameters.forEach(function(param) {
          state.paramValues[param.idx] = param.def || 0;
        });
      });
    }
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      renderKnobControls(track, def);
      renderTrackInfoBar(track, def);
    }
  }

  function refreshTrackInfoBar() {
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) renderTrackInfoBar(track, state.activeMacroDef);
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.performer = {
    init: init,
    state: state,
    setMacroDef: setMacroDef,
    refreshTrackInfoBar: refreshTrackInfoBar,
    renderKnobControls: function() {
      var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
      if (track) renderKnobControls(track, state.activeMacroDef);
    },
    exportAllPresets: exportAllPresets,
    importPresetFile: importPresetFile,
    savePresetDialog: savePresetDialog,
    loadPreset: loadPreset,
    sendParameterUpdate: sendParameterUpdate,
    reload: function() {
      if (state.activeTrack >= 0) {
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) onTrackSelected(state.activeTrack, track);
      }
    },
  };

})();

// ── designer.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Macro Definition Editor (Designer)
//
// Manages macro definition editing in the unified view.
// Renders the definition list into the sidebar (#definition-list)
// and the Macro Builder into the center panel (#macro-builder-section).
//
// The Knob Preview is handled by performer.js using the shared
// renderKnobGroups() function — no longer duplicated here.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    selectedDefId: null,
    editDef: null,
    activeTrack: -1,
    trackMachines: [],
    activeMachine: '',
    dirty: false,
    initialized: false,
  };

  // ─── UI Type Lookup ────────────────────────────────────────
  // Machine-specific CC id → {ui, curve} overrides — shared by
  // createOneToOneMapping() and the per-knob auto-select logic.
  var machineUiMap = {
    ro:  { bank: {ui:'samplebank'}, slice: {ui:'sampleslice'}, start: {ui:'sampleoffset'}, end: {ui:'sampleoffset'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, type: {ui:'filtertype'}, bitcr: {ui:'bignum'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'}, speed: {ui:'bignum'}, pitch: {ui:'bignum'}, loop: {ui:'bignum'}, pingpong: {ui:'bignum'}, ppstart: {ui:'sampleoffset'}, eg2fm: {ui:'envamount'}, tsmode: {ui:'bignum'}, tsamt: {ui:'envamount'} },
    db:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, dirt: {ui:'noise'}, 'fm-env': {ui:'envamount'}, 'fm-decay': {ui:'envdecay',curve:'exp'}, 'fm-accent': {ui:'envamount'} },
    ds:  { freq: {ui:'freq',curve:'log'}, decay: {ui:'envdecay',curve:'exp'}, fm: {ui:'shape'}, snap: {ui:'noise'}, accent: {ui:'envamount'} },
    as:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, snap: {ui:'noise'}, accent: {ui:'envamount'} },
    ab:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, 'a-fm': {ui:'shape3'}, 's-fm': {ui:'shape2'}, accent: {ui:'envamount'} },
    hh1: { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, noise: {ui:'distortion'}, accent: {ui:'envamount'} },
    hh2: { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, noise: {ui:'distortion'}, accent: {ui:'envamount'} },
    rs:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, noise: {ui:'distortion'}, accent: {ui:'envamount'} },
    cl:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, scale: {ui:'noise'} },
    fmb: { 'f-b': {ui:'shape2'}, 'd-b': {ui:'shape2',curve:'exp'}, 'f-m': {ui:'shape3'}, 'd-m': {ui:'shape3',curve:'exp'}, 'b-m': {ui:'shape2'}, 'a-f': {ui:'shape3'}, 'd-f': {ui:'shape',curve:'exp'}, i: {ui:'noise'} },
    mo:  { shape: {ui:'shape'}, p0: {ui:'shape3'}, p1: {ui:'shape2'}, waveshap: {ui:'distortion'}, attack: {ui:'envattackfast',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'} },
    td3: { shape: {ui:'shape'}, p0: {ui:'shape2'}, vca_d: {ui:'envdecay',curve:'exp'}, vcf_d: {ui:'envdecay',curve:'exp'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, envdec: {ui:'envdecay',curve:'exp'}, type: {ui:'filtertype'} },
    pp:  { detune: {ui:'distortion'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, type: {ui:'filtertype'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'}, release: {ui:'envdecay',curve:'exp'} },
    wtosc: { type: {ui:'filtertype'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'}, release: {ui:'bignum',curve:'exp'} },
    fxmaster: { compatk: {ui:'envattack',curve:'exp'}, comprel: {ui:'envdecay',curve:'exp'}, complpf: {ui:'filtercutoff',curve:'log'} },
    fxreverb: { time: {ui:'bignum',curve:'log'}, lowpass: {ui:'bignum',curve:'log'} },
    extdrum: { note: {ui:'midinote'} },
  };

  // Generic keyword fallback for machines/CCs not in the table above
  var uiKeywordMap = [
    { re: /bank/i,    ui: 'samplebank' },
    { re: /slice/i,   ui: 'sampleslice' },
    { re: /offset|start|end/i, ui: 'sampleoffset' },
    { re: /cutoff|lpf|hpf/i,   ui: 'filtercutoff', curve: 'log' },
    { re: /reso|res\b|q\b/i,   ui: 'filterq' },
    { re: /ftype|filtertype|type/i, ui: 'filtertype' },
    { re: /attack|atk/i,       ui: 'envattack', curve: 'exp' },
    { re: /decay|dec\b/i,      ui: 'envdecay', curve: 'exp' },
    { re: /release|rel\b/i,    ui: 'envdecay', curve: 'exp' },
    { re: /amount|depth|amt/i,  ui: 'envamount' },
    { re: /freq|frequency|pitch|tune/i, ui: 'freq', curve: 'log' },
    { re: /note/i,              ui: 'midinote' },
    { re: /noise/i,             ui: 'noise' },
    { re: /shape|wave/i,        ui: 'shape' },
    { re: /dist|drive|dirt/i,   ui: 'distortion' },
  ];

  // Lookup ui+curve for a given machine + cc id. Uses machineUiMap first,
  // then falls back to generic keyword matching on the CC id string.
  function lookupUiType(machineId, ccId) {
    var mMap = machineUiMap[machineId];
    if (mMap && mMap[ccId]) return mMap[ccId];
    // Generic keyword fallback
    for (var i = 0; i < uiKeywordMap.length; i++) {
      if (uiKeywordMap[i].re.test(ccId)) {
        var r = { ui: uiKeywordMap[i].ui };
        if (uiKeywordMap[i].curve) r.curve = uiKeywordMap[i].curve;
        return r;
      }
    }
    return { ui: 'bignum' };
  }

  // ─── API Helpers ──────────────────────────────────────────

  function apiGet(url) {
    return fetch(url).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }

  function apiPost(url, data) {
    return fetch(url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data),
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }

  // ─── Track Selection (from shared track tabs) ────────────

  function onTrackSelected(idx, track) {
    state.activeTrack = idx;
    state.trackMachines = S.getTrackMachines(track);
    state.selectedDefId = null;
    state.editDef = null;
    state.dirty = false;

    // Check boot default to auto-select the right machine and macro
    var bootMacroId = null;
    if (S.data.trackDefaults && S.data.trackDefaults.tracks) {
      var tdEntry = S.data.trackDefaults.tracks.find(function(t) { return t.index === idx; });
      if (tdEntry && tdEntry.preset) {
        var bootPreset = S.data.soundPresets.find(function(p) { return p.id === tdEntry.preset; });
        if (bootPreset) {
          bootMacroId = bootPreset.macro;
          var bootDef = S.data.macroDefs.find(function(d) { return d.id === bootMacroId; });
          if (bootDef && state.trackMachines.indexOf(bootDef.machine) !== -1) {
            state.activeMachine = bootDef.machine;
          }
        }
      }
    }
    if (!state.activeMachine) {
      state.activeMachine = state.trackMachines.length > 0 ? state.trackMachines[0] : '';
    }

    renderDefinitionList();
    renderMacroBuilderSection();

    // Auto-select boot default's macro if available, otherwise first
    var filteredDefs = getFilteredDefs();
    var targetDef = null;
    if (bootMacroId) {
      targetDef = filteredDefs.find(function(d) { return d.id === bootMacroId; });
    }
    if (!targetDef && filteredDefs.length > 0) {
      targetDef = filteredDefs[0];
    }
    if (targetDef) {
      selectMacroDefinition(targetDef.id);
    }
  }

  // Called by app.js when performer changes the machine select
  function onMachineChanged(machineId) {
    state.activeMachine = machineId;
    state.selectedDefId = null;
    state.editDef = null;
    state.dirty = false;

    renderDefinitionList();
    renderMacroBuilderSection();

    var filteredDefs = getFilteredDefs();
    if (filteredDefs.length > 0) {
      selectMacroDefinition(filteredDefs[0].id);
    }
  }

  function getFilteredDefs() {
    if (!state.trackMachines || state.trackMachines.length === 0) return [];
    if (state.activeMachine) {
      return S.data.macroDefs.filter(function(d) {
        return d.machine === state.activeMachine;
      });
    }
    return S.data.macroDefs.filter(function(d) {
      return state.trackMachines.indexOf(d.machine) !== -1;
    });
  }

  // ─── Param Helpers ─────────────────────────────────────────

  function is14bit(m) {
    return m && (m.bits === 14 || m.type === 'nrpm');
  }

  function findParamByIdx(idx) {
    if (!state.editDef || !state.editDef.groups) return null;
    for (var g = 0; g < state.editDef.groups.length; g++) {
      var params = state.editDef.groups[g].parameters || [];
      for (var p = 0; p < params.length; p++) {
        if (params[p].idx === idx) return params[p];
      }
    }
    return null;
  }

  function isParamUsedBy14bit(paramIdx, excludeMi) {
    if (!state.editDef) return false;
    return (state.editDef.mapping || []).some(function(m, i) {
      if (i === excludeMi) return false;
      if (!is14bit(m)) return false;
      return (m.add || []).some(function(a) { return a.src === paramIdx; });
    });
  }

  // ─── Definition List (left sidebar — Macros tab) ──────────

  function renderDefinitionList() {
    var container = document.getElementById('definition-list');
    if (!container) return;

    var filteredDefs = getFilteredDefs();

    if (state.trackMachines.length === 0) {
      container.innerHTML =
        '<div class="empty-state" style="padding:1.5rem;">' +
        '<sl-icon name="hdd" style="font-size:1.5rem;"></sl-icon>' +
        '<p style="font-size:0.78rem;">Select a track to see macro definitions</p>' +
        '</div>';
      return;
    }

    var html = '';

    // Create New button
    html += '<button class="sidebar-action-btn primary" id="create-def-btn" style="margin:0.4rem 0.65rem;width:calc(100% - 1.3rem);">';
    html += '<sl-icon name="plus-lg"></sl-icon> New Macro';
    html += '</button>';
    html += '<hr style="border:none;border-top:1px solid var(--sl-color-neutral-200);margin:0.5rem 0.65rem;">';

    if (filteredDefs.length === 0) {
      html += '<div style="padding:0.5rem 0.85rem;opacity:0.5;font-size:0.75rem;">No definitions yet</div>';
    }

    var F = window.TBD.factory;

    filteredDefs.forEach(function(def) {
      var isActive = state.selectedDefId === def.id;
      var isFactory = F && F.isFactoryDefinition(def.id);
      var isFactoryUnlocked = F && F.isUnlocked && F.isUnlocked();
      html += '<div class="preset-item' + (isActive ? ' active' : '') + '" data-def-id="' + S.esc(def.id) + '">';
      if (isFactory) {
        html += '<sl-icon name="lock" style="font-size:0.65rem;opacity:0.45;flex-shrink:0;margin-right:0.25rem;" title="Factory template' + (isFactoryUnlocked ? ' — editing unlocked' : ' — clone to edit') + '"></sl-icon>';
      }
      html += '<span class="preset-item-name" title="' + S.esc(def.name || def.id) + '">' + S.esc(def.name || def.id) + '</span>';
      html += '<span class="preset-item-machine">' + S.esc(def.id) + '</span>';
      if (!isFactory || isFactoryUnlocked) {
        html += '<button class="preset-item-delete" data-delete-def-id="' + S.esc(def.id) + '" title="Delete definition">';
        html += '<sl-icon name="trash3"></sl-icon>';
        html += '</button>';
      }
      html += '</div>';
    });

    container.innerHTML = html;
  }

  function setupDefinitionListEvents() {
    var container = document.getElementById('definition-list');
    if (!container) return;

    container.addEventListener('click', function(e) {
      if (e.target.id === 'create-def-btn' || e.target.closest('#create-def-btn')) {
        createNewDefinition();
        return;
      }
      // Handle delete button click
      var deleteBtn = e.target.closest('.preset-item-delete');
      if (deleteBtn) {
        e.stopPropagation();
        var defId = deleteBtn.getAttribute('data-delete-def-id');
        if (defId) deleteDefinition(defId);
        return;
      }
      var item = e.target.closest('.preset-item');
      if (!item) return;
      var defId = item.getAttribute('data-def-id');
      if (!defId) return;
      selectMacroDefinition(defId);
    });
  }

  // ─── Select / Edit a Macro Definition ────────────────────

  function selectMacroDefinition(defId) {
    var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
    if (!def) return;

    state.selectedDefId = defId;
    state.editDef = JSON.parse(JSON.stringify(def));
    state.dirty = false;

    ensureGroupStructure(state.editDef);

    // Update list active state
    document.querySelectorAll('#definition-list .preset-item').forEach(function(item) {
      item.classList.toggle('active', item.getAttribute('data-def-id') === defId);
    });

    renderMacroBuilderSection();
  }

  function createNewDefinition() {
    var old = document.getElementById('new-macro-dialog');
    if (old) old.remove();

    var defaultMachine = state.activeMachine || (state.trackMachines.length > 0 ? state.trackMachines[0] : '');
    var machInfo = S.getMachineInfo(defaultMachine);
    var machineName = machInfo ? machInfo.name : defaultMachine;

    // Get available machines from current track
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    var availMachines = track ? S.getTrackMachines(track) : [];
    if (availMachines.length === 0 && defaultMachine) availMachines = [defaultMachine];

    // Dialog state — track selections via sl-change events
    // (reading sl-select.value at click time is unreliable with Shoelace)
    var selectedMachine = defaultMachine;
    var selectedBaseId = '';

    function getMacrosForMachine(machId) {
      return S.data.macroDefs.filter(function(d) { return d.machine === machId; });
    }

    function buildBaseOptions(machId) {
      var macros = getMacrosForMachine(machId);
      var F = window.TBD.factory;
      var h = '<sl-option value="">Empty \u2014 start from scratch</sl-option>';
      macros.forEach(function(m) {
        var badge = (F && F.isFactoryDefinition(m.id)) ? '\ud83d\udd12 ' : '';
        h += '<sl-option value="' + S.esc(m.id) + '">' + badge + S.esc(m.name || m.id) + '</sl-option>';
      });
      return h;
    }

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'new-macro-dialog';
    dialog.label = 'New Macro';
    dialog.setAttribute('style', '--width:28rem;');

    var html = '';
    html += '<div style="display:flex;flex-direction:column;gap:0.75rem;">';
    // Machine selector
    if (availMachines.length > 1) {
      html += '<sl-select id="new-macro-machine" label="Machine" value="' + S.esc(defaultMachine) + '" hoist>';
      availMachines.forEach(function(machId) {
        var info = S.getMachineInfo(machId);
        var label = info ? info.name : machId;
        html += '<sl-option value="' + S.esc(machId) + '">' + S.esc(label) + '</sl-option>';
      });
      html += '</sl-select>';
    } else {
      html += '<div class="save-preset-context">';
      html += '<strong>' + S.esc(machineName) + '</strong>';
      html += ' <span style="opacity:0.5;">(' + S.esc(defaultMachine) + ')</span>';
      html += '</div>';
    }
    // Name input
    html += '<sl-input id="new-macro-name" label="Macro Name" placeholder="e.g. Phat Punch" required autofocus></sl-input>';
    // Based on dropdown
    html += '<sl-select id="new-macro-base" label="Based on" value="" hoist help-text="Start empty or clone an existing macro as your foundation">';
    html += buildBaseOptions(defaultMachine);
    html += '</sl-select>';
    html += '</div>';

    dialog.innerHTML = html;

    // Wire sl-change events to track selections reliably
    var machineSelectEl = dialog.querySelector('#new-macro-machine');
    if (machineSelectEl) {
      machineSelectEl.addEventListener('sl-change', function() {
        selectedMachine = machineSelectEl.value;
        // Rebuild "Based on" options for the new machine
        var oldBase = dialog.querySelector('#new-macro-base');
        if (oldBase) {
          var newBase = document.createElement('sl-select');
          newBase.id = 'new-macro-base';
          newBase.label = 'Based on';
          newBase.value = '';
          newBase.setAttribute('hoist', '');
          newBase.setAttribute('help-text', 'Start empty or clone an existing macro as your foundation');
          newBase.innerHTML = buildBaseOptions(selectedMachine);
          oldBase.replaceWith(newBase);
          selectedBaseId = '';
          newBase.addEventListener('sl-change', function() {
            selectedBaseId = newBase.value;
          });
        }
      });
    }
    var baseSelectEl = dialog.querySelector('#new-macro-base');
    if (baseSelectEl) {
      baseSelectEl.addEventListener('sl-change', function() {
        selectedBaseId = baseSelectEl.value;
      });
    }

    // Footer buttons
    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var createBtn = document.createElement('sl-button');
    createBtn.setAttribute('slot', 'footer');
    createBtn.setAttribute('variant', 'primary');
    createBtn.innerHTML = '<sl-icon name="plus-lg" slot="prefix"></sl-icon> Create Macro';

    createBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#new-macro-name');
      var name = (nameInput.value || '').trim();

      if (!name) {
        nameInput.setAttribute('help-text', 'Please enter a name');
        nameInput.focus();
        return;
      }

      var machine = selectedMachine || defaultMachine;

      // Auto-generate ID: machine prefix + slugified name
      var machinePrefix = machine ? (machine.substring(0, 2) + '-') : '';
      var slug = name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
      var autoId = machinePrefix + slug;

      if (selectedBaseId) {
        // Clone existing macro
        var sourceDef = S.data.macroDefs.find(function(d) { return d.id === selectedBaseId; });
        if (sourceDef) {
          state.editDef = JSON.parse(JSON.stringify(sourceDef));
          state.editDef.id = autoId;
          state.editDef.name = name;
          state.editDef.machine = machine;
          ensureGroupStructure(state.editDef);
        } else {
          S.toast('Source macro not found', 'danger', 2000);
          return;
        }
      } else {
        // Start from scratch
        state.editDef = {
          id: autoId,
          name: name,
          machine: machine,
          volmult: 1.0,
          groups: [],
          mapping: [],
        };
        ensureGroupStructure(state.editDef);
      }

      // Auto-save immediately so the macro exists on disk
      createBtn.setAttribute('loading', '');
      var cleanDef = cleanDefinitionForSave(state.editDef);
      var jsonStr = JSON.stringify(cleanDef, null, 2);
      var filePath = 'macros/' + state.editDef.id + '.json';

      fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: jsonStr,
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        state.selectedDefId = state.editDef.id;
        state.dirty = false;
        return S.reloadFirmwareMacros(state.editDef.id).then(function() {
          return S.reloadMacroData();
        });
      }).then(function() {
        renderDefinitionList();
        renderMacroBuilderSection();
        switchToMacroBuilder();
        S.toast('Created: ' + name, 'success', 2000);
        dialog.hide();
      }).catch(function(err) {
        createBtn.removeAttribute('loading');
        S.toast('Save failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(createBtn);
    document.body.appendChild(dialog);

    // Prevent hoisted sl-select overlay clicks from closing the dialog
    dialog.addEventListener('sl-request-close', function(e) {
      if (e.detail.source === 'overlay') e.preventDefault();
    });

    // Guard: only remove on dialog's own hide, not bubbled sl-after-hide from child sl-selects
    dialog.addEventListener('sl-after-hide', function(e) {
      if (e.target !== dialog) return;
      dialog.remove();
    });

    requestAnimationFrame(function() {
      dialog.show();
    });
  }

  // Helper to switch to Macro Builder sub-tab
  function switchToMacroBuilder() {
    var subtabs = document.querySelectorAll('.center-subtab');
    subtabs.forEach(function(tab) {
      var isBuilder = tab.getAttribute('data-subtab') === 'macro-builder';
      tab.classList.toggle('active', isBuilder);
    });
    // Update visibility
    var knobControls = document.getElementById('knob-controls');
    var macroBuilder = document.getElementById('macro-builder-section');
    var knobPreviewExtras = document.getElementById('knob-preview-extras');
    if (knobControls) knobControls.classList.add('hidden');
    if (macroBuilder) macroBuilder.classList.remove('hidden');
    if (knobPreviewExtras) knobPreviewExtras.classList.add('hidden');
  }

  function ensureGroupStructure(def) {
    if (!def.groups) def.groups = [];
    while (def.groups.length < 6) {
      def.groups.push({
        name: 'Page ' + (def.groups.length + 1),
        parameters: [],
      });
    }
    var runningIdx = 0;
    def.groups.forEach(function(group) {
      if (!group.parameters) group.parameters = [];
      group.parameters.forEach(function(p) {
        if (p.idx === undefined) {
          p.idx = runningIdx;
        }
        runningIdx = Math.max(runningIdx, p.idx + 1);
      });
    });
    if (!def.mapping) def.mapping = [];
  }

  /**
   * Re-index all parameters to contiguous 0..N-1 and update mapping src references.
   * Call after removing knobs, on import, or before save to ensure clean idx values.
   */
  function reindexParameters(def) {
    var oldToNew = {};
    var newIdx = 0;
    (def.groups || []).forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        oldToNew[p.idx] = newIdx;
        p.idx = newIdx;
        newIdx++;
      });
    });
    // Update mapping src references
    (def.mapping || []).forEach(function(m) {
      (m.add || []).forEach(function(a) {
        if (oldToNew[a.src] !== undefined) {
          a.src = oldToNew[a.src];
        }
      });
    });
  }

  /**
   * Produce a clean copy of a definition for saving/export.
   * - Re-indexes parameters to contiguous 0..N-1
   * - Strips empty trailing groups (keeps only groups that have parameters)
   * - Removes curve:'linear' from parameters (linear is the default)
   */
  function cleanDefinitionForSave(def) {
    var clean = JSON.parse(JSON.stringify(def));
    // Validate and clamp volmult
    var v = parseFloat(clean.volmult);
    if (isNaN(v) || v < 0.1) v = 1.0;
    if (v > 4.0) v = 4.0;
    clean.volmult = Math.round(v * 10) / 10;
    // Strip empty trailing groups
    while (clean.groups.length > 0 &&
           (!clean.groups[clean.groups.length - 1].parameters ||
            clean.groups[clean.groups.length - 1].parameters.length === 0)) {
      clean.groups.pop();
    }
    // Re-index to close any gaps from removed knobs
    reindexParameters(clean);
    // Remove default curve from parameters
    clean.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        if (p.curve === 'linear') delete p.curve;
      });
    });
    // ─────────────────────────────────────────────────────────────────────
    // Pico-firmware invariant guard: every mapping entry MUST carry an
    // explicit "type" field ("cc" or "nrpm") and that type MUST match the
    // same ctrl's declared type in synthdefinitions.json for this machine.
    // A missing type silently defaults to CC on the Pico and regresses
    // other machines when the synthdef actually declares the ctrl as NRPM
    // (see tbd-pico-seq3/docs/architecture/macro-system.md, the
    // "macro-vs-synthdef type agreement" invariant).
    //
    // EXCEPTION — identity mappings on performance macros. A mapping entry
    // whose ctrl == (paramIdx + 8) for some macro param idx that's ALSO
    // used as a `src` in another mapping is a "source knob identity"
    // mapping. The wire goes to the P4 MacroTranslator storage slot
    // (trackParameterValues[idx]) and is never dispatched to the DSP
    // listener directly — only the computeAndDispatch fan-out mappings
    // hit the DSP. For these identity mappings, the macro author may
    // declare type:nrpm even when synthdef says cc, to carry 14-bit
    // (hi-res) knob values into the source storage. We preserve the
    // authored type in that case. See tbd-pico-seq3/docs/architecture/
    // macro-system.md "Hi-res performance-macro source knobs".
    // ─────────────────────────────────────────────────────────────────────
    var machineInfo = clean.machine ? S.getMachineInfo(clean.machine) : null;
    var ctrlTypeLookup = {};
    if (machineInfo && machineInfo.parameters) {
      machineInfo.parameters.forEach(function(p) {
        if (typeof p.ctrl === 'number' && (p.type === 'cc' || p.type === 'nrpm')) {
          ctrlTypeLookup[p.ctrl] = p.type;
        }
      });
    }
    // Collect the set of idx values that are used as `src` in any mapping's
    // add[] array — these are the "macro source knob" indices whose
    // identity mappings (ctrl == idx + 8) are eligible for the exception.
    var srcIdxSet = {};
    (clean.mapping || []).forEach(function(m) {
      (m.add || []).forEach(function(a) {
        if (typeof a.src === 'number') srcIdxSet[a.src] = true;
      });
    });
    // Collect all paramIdx declared in groups (so we can tell "ctrl idx+8"
    // refers to a real macro source knob and not just any ctrl number).
    var paramIdxSet = {};
    (clean.groups || []).forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        if (typeof p.idx === 'number') paramIdxSet[p.idx] = true;
      });
    });
    function isPerformanceIdentityMapping(m) {
      // ctrl matches an idx+8 of some source-used param idx
      var implicitIdx = m.ctrl - 8;
      if (!paramIdxSet[implicitIdx]) return false;
      if (!srcIdxSet[implicitIdx]) return false;
      // And: at least ONE other mapping references this idx as a src
      // (already verified by srcIdxSet) AND this mapping is distinct
      // from those fan-out destinations (this mapping itself shouldn't
      // reference implicitIdx as its own src — it's an "identity",
      // typically empty add[] or fixed start).
      var selfRefs = (m.add || []).some(function(a) { return a.src === implicitIdx; });
      return !selfRefs;
    }
    (clean.mapping || []).forEach(function(m) {
      var expected = ctrlTypeLookup[m.ctrl];
      if (isPerformanceIdentityMapping(m)) {
        // Exception: preserve author-declared type. Default to cc if
        // missing entirely (never leave a mapping without type).
        if (m.type !== 'cc' && m.type !== 'nrpm') {
          m.type = expected || 'cc';
        }
      } else if (expected) {
        // Synthdef knows this ctrl → trust synthdef, overwrite any stale value
        m.type = expected;
      } else if (m.type !== 'cc' && m.type !== 'nrpm') {
        // No synthdef match (unusual, e.g. post-machine-rename) → default to CC
        m.type = 'cc';
      }
      // Keep `bits` consistent with type — NRPM implies 14-bit wire protocol
      if (m.type === 'nrpm' && m.bits !== 14) m.bits = 14;
      if (m.type === 'cc' && m.bits) delete m.bits;
    });

    // Auto-promote source-knob identity mappings to nrpm when the source
    // param itself is declared hi-res (max > 127). Rule-of-thumb applied
    // at save time: if a user sets a macro source knob to 14-bit range,
    // the wire protocol that carries it must also be 14-bit or the top 7
    // bits get lost at the MacroTranslator storage step.
    (clean.groups || []).forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        if (typeof p.idx !== 'number') return;
        if (!srcIdxSet[p.idx]) return;       // not a source knob
        if ((p.max || 127) <= 127) return;   // not hi-res
        // Find the identity mapping for this idx (ctrl == idx + 8)
        var identity = (clean.mapping || []).find(function(m) {
          return m.ctrl === p.idx + 8 && isPerformanceIdentityMapping(m);
        });
        if (identity && identity.type !== 'nrpm') {
          identity.type = 'nrpm';
          identity.bits = 14;
        }
      });
    });
    // Enforce key order: id, name, machine, volmult, groups, mapping, then rest
    var ordered = {};
    ['id', 'name', 'machine', 'volmult', 'groups', 'mapping'].forEach(function(k) {
      if (clean[k] !== undefined) ordered[k] = clean[k];
    });
    Object.keys(clean).forEach(function(k) {
      if (!(k in ordered)) ordered[k] = clean[k];
    });
    return ordered;
  }

  // ─── Definition Header (above sub-tabs) ───────────────

  // Definition header is now rendered inline in the track-info-bar by performer.js.
  // This function just keeps the legacy container hidden.
  function renderDefHeader() {
    var container = document.getElementById('macro-def-header');
    if (container) {
      container.innerHTML = '';
      container.classList.add('hidden');
    }
  }

  // ─── Sound Presets Section (Knob Preview extras) ─────────

  function renderSoundPresetsSection() {
    var container = document.getElementById('knob-preview-extras');
    if (!container) return;

    if (!state.editDef) {
      container.innerHTML = '';
      return;
    }

    var html = '<details class="dsp-collapsible" open style="margin-top:1rem;">';
    html += '<summary class="dsp-collapsible-summary">';
    html += '<sl-icon name="music-note-beamed" style="font-size:0.7rem;"></sl-icon> Presets using this Definition';
    html += '</summary>';
    html += '<div class="dsp-collapsible-body">';
    html += renderSoundPresetsForDef(state.editDef);
    html += '</div></details>';

    container.innerHTML = html;
    setupSoundPresetsSectionEvents(container);
  }

  function setupSoundPresetsSectionEvents(container) {
    var createPresetBtn = container.querySelector('.create-preset-btn');
    if (createPresetBtn) {
      createPresetBtn.addEventListener('click', function() {
        createSoundPresetForDef();
      });
    }
    container.querySelectorAll('.save-preset-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        saveEditedPreset(btn.getAttribute('data-preset-id'), container);
      });
    });
    container.querySelectorAll('.delete-preset-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        deleteSoundPreset(btn.getAttribute('data-preset-id'));
      });
    });

    // Sync slider ↔ number input for preset param values + live audition
    container.querySelectorAll('.preset-slider-input').forEach(function(slider) {
      slider.addEventListener('input', function() {
        var card = slider.closest('.sp-card');
        if (!card) return;
        var pid = slider.getAttribute('data-preset-id');
        var idx = slider.getAttribute('data-value-idx');
        var numInput = card.querySelector('.preset-value-input[data-preset-id="' + pid + '"][data-value-idx="' + idx + '"]');
        if (numInput) numInput.value = slider.value;
        // Live audition: send updated value to firmware
        sendPresetCardValueToFirmware(card, pid);
      });
    });
    container.querySelectorAll('.preset-value-input').forEach(function(numInput) {
      numInput.addEventListener('input', function() {
        var card = numInput.closest('.sp-card');
        if (!card) return;
        var pid = numInput.getAttribute('data-preset-id');
        var idx = numInput.getAttribute('data-value-idx');
        var slider = card.querySelector('.preset-slider-input[data-preset-id="' + pid + '"][data-value-idx="' + idx + '"]');
        if (slider) slider.value = numInput.value;
        // Live audition: send updated value to firmware
        sendPresetCardValueToFirmware(card, pid);
      });
    });
  }

  // Send all values from a preset card to firmware for live audition
  function sendPresetCardValueToFirmware(card, presetId) {
    var P = window.TBD.performer;
    if (!P || !P.state || P.state.activeTrack < 0 || !state.editDef) return;

    // Gather all values from the card's inputs
    var values = [];
    card.querySelectorAll('.preset-value-input[data-preset-id="' + presetId + '"]').forEach(function(input) {
      var idx = parseInt(input.getAttribute('data-value-idx'), 10);
      values[idx] = parseInt(input.value, 10) || 0;
    });

    // Fill missing indices with defaults from the definition
    if (state.editDef.groups) {
      state.editDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) {
          if (values[p.idx] === undefined) {
            values[p.idx] = p.def || 0;
          }
        });
      });
    }

    // Update performer state and send to firmware
    P.state.paramValues = values;
    if (P.sendParameterUpdate) P.sendParameterUpdate();
  }

  // ─── Macro Builder Section (center panel) ────────────────

  function renderMacroBuilderSection() {
    var container = document.getElementById('macro-builder-section');
    if (!container) return;

    // Also render the def header and sound presets sections
    renderDefHeader();
    renderSoundPresetsSection();

    // Sync performer's Knob Preview with current editDef
    if (state.editDef && window.TBD.performer && window.TBD.performer.setMacroDef) {
      window.TBD.performer.setMacroDef(state.editDef);
    }

    if (!state.editDef) {
      container.innerHTML =
        '<div class="empty-state" id="mapping-empty">' +
        '<sl-icon name="table"></sl-icon>' +
        '<h3>Select a Macro Definition</h3>' +
        '<p>Pick a definition from the sidebar to edit how DSP parameters are exposed as performer knobs.</p>' +
        '</div>';
      return;
    }

    var def = state.editDef;
    var machineInfo = S.getMachineInfo(def.machine);
    var machineParams = machineInfo ? (machineInfo.parameters || []) : [];

    var html = '';

    // 1:1 Map button + description in one row
    html += '<div class="mb-top-actions" style="display:flex;align-items:center;gap:0.75rem;flex-wrap:wrap;padding:0.5rem 0;">';
    html += '<button class="mapping-btn btn-1to1" title="Auto-create a 1:1 mapping from all machine CCs" style="flex-shrink:0;">1:1 Map</button>';
    html += '<span style="font-size:0.82rem;color:var(--sl-color-neutral-600);line-height:1.3;">';
    html += '<sl-icon name="info-circle" style="font-size:0.75rem;vertical-align:-1px;margin-right:0.2rem;"></sl-icon>';
    html += 'Define knobs (up to 6 pages \u00d7 4 knobs) and map them to DSP parameters. Use <strong>1:1 Map</strong> to auto-create all knobs, or add them manually below.';
    html += '</span>';
    html += '</div>';

    // DSP Reference (collapsible, at top of builder for quick access)
    if (machineInfo && machineParams.length > 0) {
      html += '<details class="dsp-collapsible dsp-ref-top" style="margin-bottom:0.75rem;">';
      html += '<summary class="dsp-collapsible-summary">';
      html += '<sl-icon name="hdd" style="font-size:0.7rem;"></sl-icon> DSP Reference: ' + S.esc(machineInfo.name);
      html += '</summary>';
      html += '<div class="dsp-collapsible-body">';
      html += renderDSPList(machineInfo, machineParams);
      html += '</div></details>';
    }

    // Macro builder pages and mappings
    html += renderMacroBuilder(def, machineParams);

    container.innerHTML = html;
    setupMappingEditorEvents(container);
  }

  // ─── DSP Reference List (inline collapsible) ─────────────

  function renderDSPList(machineInfo, machineParams) {
    var mappedCtrls = {};
    if (state.editDef && state.editDef.mapping) {
      state.editDef.mapping.forEach(function(m) {
        mappedCtrls[m.ctrl] = true;
      });
    }

    var html = '';
    machineParams.forEach(function(p) {
      var isMapped = mappedCtrls[p.ctrl] || false;
      html += '<div class="dsp-param-row' + (isMapped ? ' mapped' : '') + '" data-ctrl="' + p.ctrl + '">';
      html += '<span class="dsp-param-index">CC ' + p.ctrl + '</span>';
      html += '<span class="dsp-param-name">' + S.esc(p.name) + '</span>';
      html += '<span class="dsp-param-value">def: ' + (p.def || 0) + '</span>';
      if (isMapped) {
        html += '<sl-icon name="check2-square" class="dsp-param-mapped-icon" title="Mapped"></sl-icon>';
      }
      html += '</div>';
    });
    return html;
  }

  // ── Render: Macro Builder (merged Parameter Groups + Output Mappings) ──

  function renderMacroBuilder(def, machineParams) {
    var mappings = def.mapping || [];
    var DH = window.TBD && window.TBD.displayHints;
    var html = '';

    // Build lookup maps
    var paramsByIdx = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramsByIdx[p.idx] = p;
      });
    });

    var ccLookup = {};
    machineParams.forEach(function(p) {
      ccLookup[p.ctrl] = p;
    });

    // ── Group mappings by source knob ──
    var paramMappings = {};
    var constants = [];
    var multiSourceMappings = [];

    mappings.forEach(function(m, mi) {
      var sources = m.add || [];
      if (sources.length === 0) {
        constants.push({ mi: mi, mapping: m });
      } else if (sources.length === 1) {
        var src = sources[0].src;
        if (!paramMappings[src]) paramMappings[src] = [];
        paramMappings[src].push({ mi: mi, ai: 0, mapping: m });
      } else {
        multiSourceMappings.push({ mi: mi, mapping: m });
        sources.forEach(function(a, ai) {
          if (!paramMappings[a.src]) paramMappings[a.src] = [];
          paramMappings[a.src].push({ mi: mi, ai: ai, mapping: m });
        });
      }
    });

    function fmtCC(ctrl) {
      return 'CC\u2009' + String(ctrl).padStart(2, '0');
    }

    function getSemanticInfo(ctrl, rangeLow, rangeHigh, maxCC, srcParam) {
      var mp = ccLookup[ctrl];
      if (!mp || !DH) return { unit: '', rangeStr: '', hint: null };
      var paramId = (def.machine || '') + '_' + (mp.id || '').replace(/-/g, '_');
      // Merge macro-side srcParam's `ui` into the hint query so per-ui-type
      // overrides (e.g. "envattackfast" → 0.5 ms..1 s) take precedence over
      // the generic DSP-side lookups. srcParam is the entry in
      // def.groups[].parameters[] matching this mapping's add[].src idx.
      var hintParam = mp;
      if (srcParam && srcParam.ui) {
        hintParam = Object.assign({}, mp, { ui: srcParam.ui });
      }
      var hint = DH.resolveHint(paramId, mp.name, hintParam);
      if (!hint) return { unit: '', rangeStr: '', hint: null };
      var rawMax = maxCC || 127;
      var physLow = DH.rawToDisplay(rangeLow, 0, rawMax, hint);
      var physHigh = DH.rawToDisplay(rangeHigh, 0, rawMax, hint);
      var fmtLow = DH.formatDisplayValue(physLow, hint);
      var fmtHigh = DH.formatDisplayValue(physHigh, hint);
      return {
        unit: hint.unit || '',
        rangeStr: fmtLow + ' \u2192 ' + fmtHigh,
        hint: hint,
        scale: hint.scale || 'lin'
      };
    }

    function renderKnob(paramIdx, param, size, color) {
      var val = param ? (param.def || 0) : 0;
      var mn = param ? (param.min || 0) : 0;
      var mx = param ? (param.max || 127) : 127;
      var name = param ? (param.name || ('P' + paramIdx)) : ('P' + paramIdx);
      var h = '<div class="om-knob om-knob-interactive" data-value="' + val + '" data-min="' + mn + '" data-max="' + mx + '" data-idx="' + paramIdx + '" data-color="' + color + '" title="' + S.esc(name) + ' \u2014 drag up/down">';
      h += S.renderKnobSVG({ value: val, min: mn, max: mx, color: color, size: size });
      h += '</div>';
      return h;
    }

    function computeValueDot(param, mapping, ai) {
      var addEntry = mapping.add[ai];
      if (!addEntry) return null;
      var is14 = is14bit(mapping);
      var maxCC = is14 ? 16383 : 127;
      var start = mapping.start || 0;
      var mul = addEntry.mul || 1;
      var div = addEntry.div || 1;
      var paramVal = param ? (param.def || 0) : 0;
      var ccVal = start + Math.round(paramVal * mul / div);
      ccVal = Math.max(0, Math.min(maxCC, ccVal));
      return { cc: ccVal, maxCC: maxCC };
    }

    function renderCCRow(mi, ai, m, addEntry) {
      var ctrl = m.ctrl;
      var mp = ccLookup[ctrl];
      var ccName = mp ? mp.name : '?';
      var is14 = is14bit(m);
      var maxCC = is14 ? 16383 : 127;
      var range = sourceToRange(m, ai);
      var curve = addEntry.curve || 'linear';
      var lowPct = range.low / maxCC * 100;
      var highPct = range.high / maxCC * 100;
      var srcParam = paramsByIdx[addEntry.src];
      var sem = getSemanticInfo(ctrl, range.low, range.high, maxCC, srcParam);
      var dot = computeValueDot(srcParam, m, ai);

      var r = '';
      r += '<div class="om-cc-row" data-mapping-idx="' + mi + '" data-add="' + ai + '">';
      r += '<span class="om-cc-label">' + fmtCC(ctrl) + '</span>';
      r += '<span class="om-cc-name">' + S.esc(ccName) + '</span>';
      r += '<input type="number" class="mapping-input om-range-low' + (is14 ? ' is-14bit' : '') + '" value="' + range.low + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" data-add="' + ai + '" title="Low' + (is14 ? ' (0\u201316383)' : ' (0\u2013127)') + '" />';
      r += '<div class="om-range-track" data-mapping="' + mi + '" data-add="' + ai + '">';
      r += '<div class="om-range-fill" style="left:' + lowPct + '%;width:' + (highPct - lowPct) + '%"></div>';
      r += '<div class="om-range-thumb om-thumb-low" style="left:' + lowPct + '%" data-mapping="' + mi + '" data-add="' + ai + '"></div>';
      r += '<div class="om-range-thumb om-thumb-high" style="left:' + highPct + '%" data-mapping="' + mi + '" data-add="' + ai + '"></div>';
      if (dot) {
        var dotPct = (dot.maxCC > 0) ? (dot.cc / dot.maxCC * 100) : 0;
        r += '<div class="om-value-dot" style="left:' + dotPct + '%" data-mapping="' + mi + '" data-add="' + ai + '" title="Current CC value: ' + dot.cc + '"></div>';
      }
      r += '</div>';
      r += '<input type="number" class="mapping-input om-range-high' + (is14 ? ' is-14bit' : '') + '" value="' + range.high + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" data-add="' + ai + '" title="High' + (is14 ? ' (0\u201316383)' : ' (0\u2013127)') + '" />';
      r += '<select class="mapping-select om-curve-select" data-mapping="' + mi + '" data-add="' + ai + '" title="Response curve">';
      ['linear','log','exp','scurve'].forEach(function(c) {
        r += '<option value="' + c + '"' + (curve === c ? ' selected' : '') + '>' + c + '</option>';
      });
      r += '</select>';
      r += '<label class="om-bit-toggle" title="Enable 14-bit CC (0\u201316383) for higher precision">';
      r += '<input type="checkbox" class="om-14bit-check" data-mapping="' + mi + '"' + (is14 ? ' checked' : '') + ' />';
      r += '<span>14-bit</span>';
      r += '</label>';
      r += '<button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove this CC mapping">\u00d7</button>';
      r += '</div>';

      if (sem.rangeStr) {
        r += '<div class="om-semantic-row">';
        r += '<span class="om-semantic-range">' + sem.rangeStr + '</span>';
        if (sem.scale === 'log' && curve === 'linear') {
          r += '<a class="om-scale-hint om-scale-fix" href="#" data-mapping="' + mi + '" data-add="' + ai + '" title="This DSP parameter has a logarithmic scale. Click to switch the curve to log.">\ud83d\udca1 use log curve</a>';
        }
        r += '</div>';
      }

      return r;
    }

    // ── Render page sections (groups) ──
    def.groups.forEach(function(group, gi) {
      html += '<div class="mb-page-section" data-group-idx="' + gi + '">';
      html += '<div class="mb-page-header">';
      html += '<span class="mb-page-icon"><sl-icon name="table" style="font-size:0.7rem;"></sl-icon></span>';
      html += '<span class="mb-page-label">Page ' + (gi + 1) + '</span>';
      html += '<input class="mapping-input mb-group-name" value="' + S.esc(group.name) + '" data-group="' + gi + '" placeholder="Name (optional)" />';
      html += '<span class="mb-page-info">' + (group.parameters || []).length + '/4 knobs</span>';
      html += '<div class="om-card-spacer"></div>';
      if ((group.parameters || []).length < 4) {
        html += '<button class="mapping-add-btn mb-add-knob-btn" data-group="' + gi + '" title="Add a new knob parameter">+ Add Knob</button>';
      }
      html += '</div>';

      html += '<div class="mb-page-content">';
      (group.parameters || []).forEach(function(param, pi) {
        var paramIdx = param.idx;
        var entries = paramMappings[paramIdx] || [];
        var isMacro = entries.length >= 2;
        var knobColor = isMacro ? 'macro' : 'normal';

        html += '<div class="om-knob-card' + (isMacro ? ' is-macro' : '') + '" data-group="' + gi + '" data-param="' + pi + '" data-param-idx="' + paramIdx + '">';
        html += '<div class="om-knob-header">';
        html += '<span class="om-drag-handle" title="Drag to reorder">\u2AF6</span>';
        html += '<span class="om-knob-badge">Knob ' + (pi + 1) + '</span>';
        html += '<div class="om-knob-cell">';
        html += '<input class="mapping-input mb-param-name" value="' + S.esc(param.name) + '" data-group="' + gi + '" data-param="' + pi + '" placeholder="Knob name" />';
        html += renderKnob(paramIdx, param, 64, knobColor);
        html += '<span class="om-knob-value' + (isMacro ? ' is-macro' : '') + '">' + (param.def || 0) + '</span>';
        html += '</div>';
        html += '<div class="om-card-spacer"></div>';
        if (isMacro) {
          html += '<sl-badge class="om-macro-badge" variant="warning" size="small">MACRO \u00b7 ' + entries.length + '</sl-badge>';
        }
        html += '<select class="mapping-select mapping-add-cc-for-knob" data-src-idx="' + paramIdx + '" title="Map this knob to another CC">';
        html += '<option value="">+ map to CC\u2026</option>';
        machineParams.forEach(function(mp) {
          html += '<option value="' + mp.ctrl + '">' + fmtCC(mp.ctrl) + ' ' + S.esc(mp.name) + '</option>';
        });
        html += '</select>';
        html += '<button class="mapping-remove-btn mb-remove-knob-btn" data-group="' + gi + '" data-param="' + pi + '" data-param-idx="' + paramIdx + '" title="Remove this knob">\u00d7</button>';
        html += '</div>';

        // Properties row
        var is14bitParam = (param.max || 127) > 127;
        var propCls = 'mb-prop' + (is14bitParam ? ' is-14bit' : '');
        html += '<div class="mb-props-row">';
        html += '<label class="' + propCls + '"><span>def</span><input type="number" class="mapping-input mb-prop-def" value="' + (param.def || 0) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="' + propCls + '"><span>min</span><input type="number" class="mapping-input mb-prop-min" value="' + (param.min || 0) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="' + propCls + '"><span>max</span><input type="number" class="mapping-input mb-prop-max" value="' + (param.max || 127) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="' + propCls + '"><span>res</span><input type="number" class="mapping-input mb-prop-res" value="' + (param.res || 64) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>ui</span><select class="mapping-select mb-prop-ui" data-group="' + gi + '" data-param="' + pi + '">';
        ['bignum', 'slider', 'toggle', 'selector', 'knob', 'freq', 'midinote', 'shape', 'shape2', 'shape3', 'noise', 'distortion', 'envattack', 'envattackfast', 'envdecay', 'envamount', 'filtercutoff', 'filterq', 'filtertype', 'samplebank', 'sampleslice', 'sampleoffset'].forEach(function(ui) {
          html += '<option value="' + ui + '"' + (param.ui === ui ? ' selected' : '') + '>' + ui + '</option>';
        });
        html += '</select></label>';
        html += '</div>';

        // CC mapping rows
        html += '<div class="om-knob-body">';
        if (entries.length === 0) {
          html += '<div class="mb-no-mappings">No CC mappings \u2014 use "+ map to CC\u2026" above</div>';
        }
        entries.forEach(function(entry) {
          html += renderCCRow(entry.mi, entry.ai, entry.mapping, entry.mapping.add[entry.ai]);
        });
        html += '</div>';
        html += '</div>';
      });

      if (!group.parameters || group.parameters.length === 0) {
        html += '<div class="mb-empty-group">No knobs in this page \u2014 click "+ Add Knob" to create one</div>';
      }
      html += '</div>';
      html += '</div>';
    });

    // Add Page button
    if (def.groups.length < 6) {
      html += '<div style="text-align:center;margin:0.6rem 0;">';
      html += '<button class="mapping-add-btn mb-add-group-btn" title="Add a new knob page (up to 6)">+ Add Page</button>';
      html += '</div>';
    }

    // Constants (locked parameters)
    if (constants.length > 0) {
      html += '<div class="om-card om-constants-card">';
      html += '<div class="om-card-header">';
      html += '<sl-icon name="lock" style="font-size:0.85rem;color:var(--sl-color-neutral-500);"></sl-icon>';
      html += '<span class="om-cc-name" style="font-weight:700;">Locked Parameters</span>';
      html += '<div class="om-card-spacer"></div>';
      html += '<sl-badge variant="neutral" size="small">' + constants.length + ' locked</sl-badge>';
      html += '</div>';
      html += '<div class="om-card-body">';

      constants.forEach(function(entry) {
        var m = entry.mapping;
        var mi = entry.mi;
        var ctrl = m.ctrl;
        var mp = ccLookup[ctrl];
        var ccName = mp ? mp.name : '?';
        var is14 = is14bit(m);
        var maxCC = is14 ? 16383 : 127;
        var fixedVal = m.start || 0;
        var fixedPct = fixedVal / maxCC * 100;
        var sem = getSemanticInfo(ctrl, fixedVal, fixedVal, maxCC);

        html += '<div class="om-constant-row" data-mapping-idx="' + mi + '">';
        html += '<span class="om-cc-label">' + fmtCC(ctrl) + '</span>';
        html += '<span class="om-cc-name">' + S.esc(ccName) + '</span>';
        html += '<input type="number" class="mapping-input om-fixed-input' + (is14 ? ' is-14bit' : '') + '" value="' + fixedVal + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" />';
        html += '<div class="om-range-track" title="Fixed CC value">';
        html += '<div class="om-range-mark" style="left:' + fixedPct + '%"></div>';
        html += '</div>';
        if (sem.rangeStr) {
          html += '<span class="om-semantic-val">' + S.esc(sem.rangeStr.split(' \u2192 ')[0]) + '</span>';
        }
        html += '<label class="om-bit-toggle" title="Enable 14-bit CC">';
        html += '<input type="checkbox" class="om-14bit-check" data-mapping="' + mi + '"' + (is14 ? ' checked' : '') + ' />';
        html += '<span>14-bit</span>';
        html += '</label>';
        html += '<button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove"><sl-icon name="x-lg"></sl-icon></button>';
        html += '</div>';
      });

      html += '</div>';
      html += '</div>';
    }

    // Unmapped CC add dropdown
    html += '<div style="margin-top:0.5rem;text-align:center;">';
    html += '<select class="mapping-select add-unmapped-cc-select" title="Add a constant (locked) CC mapping">';
    html += '<option value="">+ add locked CC\u2026</option>';
    machineParams.forEach(function(mp) {
      var alreadyMapped = mappings.some(function(m) { return m.ctrl === mp.ctrl; });
      if (!alreadyMapped) {
        html += '<option value="' + mp.ctrl + '">' + fmtCC(mp.ctrl) + ' ' + S.esc(mp.name) + '</option>';
      }
    });
    html += '</select>';
    html += '</div>';

    return html;
  }

  // ── Helpers: range ↔ start/mul/div conversion ──

  function sourceToRange(mapping, addIdx) {
    var add = (mapping.add || [])[addIdx];
    var maxCC = is14bit(mapping) ? 16383 : 127;
    if (!add) return { low: mapping.start || 0, high: mapping.start || 0 };
    var mul = add.mul || 1;
    var div = add.div || 1;
    var singleSource = (mapping.add || []).length === 1;
    if (singleSource) {
      var low = mapping.start || 0;
      var high = low + Math.round(maxCC * mul / div);
      return { low: Math.max(0, Math.min(maxCC, low)), high: Math.max(0, Math.min(maxCC, high)) };
    } else {
      var maxC = Math.round(maxCC * mul / div);
      return { low: 0, high: Math.max(0, Math.min(maxCC, maxC)), base: mapping.start || 0 };
    }
  }

  function rangeToSource(mapping, addIdx, low, high) {
    var maxCC = is14bit(mapping) ? 16383 : 127;
    var singleSource = (mapping.add || []).length === 1;
    if (singleSource) {
      mapping.start = low;
      mapping.add[addIdx].mul = high - low;
      mapping.add[addIdx].div = maxCC;
    } else {
      mapping.add[addIdx].mul = high;
      mapping.add[addIdx].div = maxCC;
    }
  }

  // ─── Sortable: Knob reordering within pages ──────────────

  var knobSortableInstances = [];

  function setupKnobSortables(container) {
    knobSortableInstances.forEach(function(s) { try { s.destroy(); } catch(e) {} });
    knobSortableInstances = [];
    if (typeof Sortable === 'undefined') return;

    container.querySelectorAll('.mb-page-content').forEach(function(pageContent) {
      var section = pageContent.closest('.mb-page-section');
      if (!section) return;
      var gi = parseInt(section.getAttribute('data-group-idx'), 10);

      var inst = Sortable.create(pageContent, {
        handle: '.om-drag-handle',
        animation: 150,
        ghostClass: 'sortable-ghost',
        chosenClass: 'sortable-chosen',
        draggable: '.om-knob-card',
        onEnd: function(evt) {
          if (evt.oldIndex !== evt.newIndex) {
            reorderKnobInGroup(gi, evt.oldIndex, evt.newIndex);
          }
        },
      });
      knobSortableInstances.push(inst);
    });
  }

  function reorderKnobInGroup(groupIdx, oldPos, newPos) {
    if (!state.editDef) return;
    var group = state.editDef.groups[groupIdx];
    if (!group || !group.parameters) return;
    var moved = group.parameters.splice(oldPos, 1)[0];
    group.parameters.splice(newPos, 0, moved);
    state.dirty = true;
    renderMacroBuilderSection();
  }

  // ── Render: Sound Presets ──

  function renderSoundPresetsForDef(def) {
    var matching = S.data.soundPresets.filter(function(p) {
      return p.macro === def.id;
    });

    var html = '';
    html += '<div class="mapping-output-header">';
    html += '<span>Presets using \u201c' + S.esc(def.name || def.id) + '\u201d (' + matching.length + ')</span>';
    html += '<button class="mapping-add-btn create-preset-btn" title="Create new sound preset">+ New Preset</button>';
    html += '</div>';

    if (matching.length === 0) {
      html += '<div style="text-align:center;opacity:0.4;padding:1rem;">No sound presets reference this definition</div>';
      return html;
    }

    var FP = window.TBD.factory;

    matching.forEach(function(preset) {
      var isFactory = FP && FP.isFactoryPreset(preset.id);
      var isUnlocked = FP && FP.isUnlocked && FP.isUnlocked();
      var isEditable = !isFactory || isUnlocked;
      html += '<div class="sp-card' + (isFactory && !isUnlocked ? ' sp-card-readonly' : '') + '" data-preset-id="' + S.esc(preset.id) + '">';

      // Card header: labeled fields + action buttons
      html += '<div class="sp-card-header">';
      html += '<div class="sp-card-fields">';
      if (isFactory && !isUnlocked) {
        html += '<span class="sp-factory-badge" title="Factory preset — unlock Factory Edit Mode to modify"><sl-icon name="lock" style="font-size:0.55rem;"></sl-icon> Factory</span>';
      }
      html += '<label class="sp-field-label">Preset Name</label>';
      html += '<input class="mapping-input sp-name-input preset-name-input" value="' + S.esc(preset.name || preset.id) + '" data-preset-id="' + S.esc(preset.id) + '" placeholder="e.g. Fat Punch"' + (!isEditable ? ' readonly' : '') + ' />';
      html += '<label class="sp-field-label" style="margin-left:0.5rem;">Group</label>';
      html += '<input class="mapping-input sp-group-input preset-group-input" value="' + S.esc(preset.group || '') + '" data-preset-id="' + S.esc(preset.id) + '" placeholder="e.g. User"' + (!isEditable ? ' readonly' : '') + ' />';
      html += '</div>';
      html += '<div class="sp-card-actions">';
      if (isEditable) {
        html += '<button class="mapping-btn save-preset-btn" data-preset-id="' + S.esc(preset.id) + '" title="Save changes"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save</button>';
        html += '<button class="mapping-btn delete-preset-btn" data-preset-id="' + S.esc(preset.id) + '" title="Delete preset"><sl-icon name="trash3" style="font-size:0.7rem;"></sl-icon> Delete</button>';
      }
      html += '</div>';
      html += '</div>';

      // Card body: render knobs using the shared knob renderer (compact)
      html += '<div class="sp-card-knobs">';
      html += S.renderKnobGroups(def, preset.values || [], { knobSize: 56 });
      html += '</div>';

      html += '</div>'; // .sp-card
    });

    return html;
  }

  // ─── Mapping Editor Events ───────────────────────────────

  function setupMappingEditorEvents(container) {
    // 1:1 mapping button
    var btn1to1 = container.querySelector('.btn-1to1');
    if (btn1to1) {
      btn1to1.addEventListener('click', function() {
        createOneToOneMapping();
      });
    }

    // Group name changes
    container.querySelectorAll('.mb-group-name').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        if (state.editDef && state.editDef.groups[gi]) {
          // Rompler macros: first page must always be called SAMPLE
          if (gi === 0 && state.editDef.machine === 'ro' && input.value !== 'SAMPLE') {
            S.toast('Rompler macros require the first page to be named SAMPLE', 'warning', 3000);
            input.value = 'SAMPLE';
            return;
          }
          state.editDef.groups[gi].name = input.value;
          state.dirty = true;
        }
      });
    });

    // Parameter property changes
    container.querySelectorAll('.mb-param-name').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        var pi = parseInt(input.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (param) { param.name = input.value; state.dirty = true; }
      });
    });
    container.querySelectorAll('.mb-prop-def, .mb-prop-min, .mb-prop-max, .mb-prop-res').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        var pi = parseInt(input.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (!param) return;
        var v = parseInt(input.value, 10) || 0;
        if (input.classList.contains('mb-prop-def')) param.def = v;
        if (input.classList.contains('mb-prop-min')) param.min = v;
        if (input.classList.contains('mb-prop-max')) param.max = v;
        if (input.classList.contains('mb-prop-res')) param.res = v;
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });
    container.querySelectorAll('.mb-prop-ui').forEach(function(select) {
      select.addEventListener('change', function() {
        var gi = parseInt(select.getAttribute('data-group'), 10);
        var pi = parseInt(select.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (param) { param.ui = select.value; state.dirty = true; }
      });
    });

    // Add knob
    container.querySelectorAll('.mb-add-knob-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        if ((state.editDef.groups[gi].parameters || []).length >= 4) return;
        var maxIdx = -1;
        state.editDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx > maxIdx) maxIdx = p.idx;
          });
        });
        state.editDef.groups[gi].parameters.push({
          idx: maxIdx + 1,
          name: 'New Knob',
          def: 0, min: 0, max: 127, res: 64, ui: 'bignum',
        });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Remove knob
    container.querySelectorAll('.mb-remove-knob-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        var pi = parseInt(btn.getAttribute('data-param'), 10);
        var paramIdx = parseInt(btn.getAttribute('data-param-idx'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        state.editDef.groups[gi].parameters.splice(pi, 1);
        if (state.editDef.mapping) {
          state.editDef.mapping = state.editDef.mapping.filter(function(m) {
            if (!m.add || m.add.length === 0) return true;
            m.add = m.add.filter(function(a) { return a.src !== paramIdx; });
            return m.add.length > 0 || (m.start !== undefined);
          });
        }
        // Re-index remaining parameters to close the gap
        reindexParameters(state.editDef);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Sortable
    setupKnobSortables(container);

    // Add page
    container.querySelectorAll('.mb-add-group-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        if (!state.editDef) return;
        if (state.editDef.groups.length >= 6) return;
        state.editDef.groups.push({
          name: 'Page ' + (state.editDef.groups.length + 1),
          parameters: [],
        });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Interactive knob drag
    container.querySelectorAll('.om-knob-interactive').forEach(function(knob) {
      var paramIdx = parseInt(knob.getAttribute('data-idx'), 10);
      var min = parseInt(knob.getAttribute('data-min'), 10) || 0;
      var max = parseInt(knob.getAttribute('data-max'), 10) || 127;
      var startY = 0;
      var startVal = 0;

      knob.addEventListener('pointerdown', function(e) {
        e.preventDefault();
        knob.classList.add('dragging');
        startY = e.clientY;
        startVal = parseInt(knob.getAttribute('data-value'), 10) || 0;

        function onMove(ev) {
          var dy = startY - ev.clientY;
          var range = max - min;
          var sensitivity = range / 200;
          var newVal = Math.round(startVal + dy * sensitivity);
          newVal = Math.max(min, Math.min(max, newVal));

          knob.setAttribute('data-value', newVal);
          var color = knob.getAttribute('data-color') || 'normal';
          var size = knob.querySelector('.knob-svg') ? parseInt(knob.querySelector('.knob-svg').getAttribute('width'), 10) : 32;
          knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: color, size: size });

          var card = knob.closest('.om-knob-card');
          if (card) {
            var valEl = card.querySelector('.om-knob-value');
            if (valEl) valEl.textContent = newVal;
            var defInput = card.querySelector('.mb-prop-def');
            if (defInput) defInput.value = newVal;

            card.querySelectorAll('.om-value-dot').forEach(function(dot) {
              var mi = parseInt(dot.getAttribute('data-mapping'), 10);
              var ai = parseInt(dot.getAttribute('data-add'), 10);
              if (!state.editDef || !state.editDef.mapping[mi]) return;
              var mapping = state.editDef.mapping[mi];
              var addEntry = mapping.add && mapping.add[ai];
              if (!addEntry) return;
              var is14 = is14bit(mapping);
              var maxCC = is14 ? 16383 : 127;
              var start = mapping.start || 0;
              var mul = addEntry.mul || 1;
              var div = addEntry.div || 1;
              var ccVal = start + Math.round(newVal * mul / div);
              ccVal = Math.max(0, Math.min(maxCC, ccVal));
              dot.style.left = (ccVal / maxCC * 100) + '%';
              dot.title = 'Current CC value: ' + ccVal;
            });
          }

          if (state.editDef) {
            state.editDef.groups.forEach(function(g) {
              (g.parameters || []).forEach(function(p) {
                if (p.idx === paramIdx) p.def = newVal;
              });
            });
            state.dirty = true;
          }
        }

        function onUp() {
          knob.classList.remove('dragging');
          document.removeEventListener('pointermove', onMove);
          document.removeEventListener('pointerup', onUp);
        }

        document.addEventListener('pointermove', onMove);
        document.addEventListener('pointerup', onUp);
      });
    });

    // Range low/high input changes
    container.querySelectorAll('.om-range-low, .om-range-high').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        var ai = parseInt(input.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var mapping = state.editDef.mapping[mi];
        var maxCC = is14bit(mapping) ? 16383 : 127;
        var row = input.closest('.om-cc-row, .om-source-row');
        var lowInput = row ? row.querySelector('.om-range-low') : null;
        var highInput = row ? row.querySelector('.om-range-high') : null;
        var low = Math.max(0, Math.min(maxCC, parseInt(lowInput ? lowInput.value : 0, 10) || 0));
        var high = Math.max(0, Math.min(maxCC, parseInt(highInput ? highInput.value : maxCC, 10) || 0));
        if (low > high) { var tmp = low; low = high; high = tmp; }
        rangeToSource(mapping, ai, low, high);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Curve select
    container.querySelectorAll('.om-curve-select').forEach(function(select) {
      select.addEventListener('change', function() {
        var mi = parseInt(select.getAttribute('data-mapping'), 10);
        var ai = parseInt(select.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var addEntry = state.editDef.mapping[mi].add[ai];
        if (addEntry) { addEntry.curve = select.value; state.dirty = true; }
      });
    });

    // Scale hint fix
    container.querySelectorAll('.om-scale-fix').forEach(function(link) {
      link.addEventListener('click', function(e) {
        e.preventDefault();
        var mi = parseInt(link.getAttribute('data-mapping'), 10);
        var ai = parseInt(link.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var addEntry = state.editDef.mapping[mi].add[ai];
        if (addEntry) { addEntry.curve = 'log'; state.dirty = true; renderMacroBuilderSection(); }
      });
    });

    // Fixed value inputs
    container.querySelectorAll('.om-fixed-input').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var maxCC = is14bit(state.editDef.mapping[mi]) ? 16383 : 127;
        state.editDef.mapping[mi].start = Math.max(0, Math.min(maxCC, parseInt(input.value, 10) || 0));
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Multi-source base start
    container.querySelectorAll('.om-multi-card .mapping-start').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var maxCC = is14bit(state.editDef.mapping[mi]) ? 16383 : 127;
        state.editDef.mapping[mi].start = Math.max(0, Math.min(maxCC, parseInt(input.value, 10) || 0));
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // 14-bit toggle
    container.querySelectorAll('.om-14bit-check').forEach(function(checkbox) {
      checkbox.addEventListener('change', function() {
        var mi = parseInt(checkbox.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var mapping = state.editDef.mapping[mi];
        if (checkbox.checked) {
          // Switching 7-bit → 14-bit: scale mapping start, scale source params up
          mapping.bits = 14;
          mapping.type = 'nrpm';
          mapping.start = Math.round((mapping.start || 0) * 16383 / 127);
          (mapping.add || []).forEach(function(a) {
            var param = findParamByIdx(a.src);
            if (param && (param.max || 127) <= 127) {
              var pOld = param.max || 127;
              param.def = Math.round((param.def || 0) * 16383 / pOld);
              param.max = 16383;
              param.res = Math.round((param.res || Math.round(pOld / 2)) * 16383 / pOld);
            }
          });
        } else {
          // Switching 14-bit → 7-bit: scale mapping start, scale source params down.
          // Mapping.type must be explicit on every entry (see Pico
          // docs/architecture/macro-system.md "macro-vs-synthdef type agreement"
          // invariant — missing type silently defaults to CC on the Pico and
          // regresses other machines). Set type:"cc" here, never delete it.
          delete mapping.bits;
          mapping.type = 'cc';
          mapping.start = Math.min(127, Math.round((mapping.start || 0) * 127 / 16383));
          (mapping.add || []).forEach(function(a) {
            if (a.mul > 127) a.mul = 127;
            // Only scale param down if no other mapping still needs 14-bit
            if (isParamUsedBy14bit(a.src, mi)) return;
            var param = findParamByIdx(a.src);
            if (param && (param.max || 127) > 127) {
              var pOld = param.max;
              param.def = Math.round((param.def || 0) * 127 / pOld);
              param.max = 127;
              param.res = Math.round((param.res || Math.round(pOld / 2)) * 127 / pOld);
              param.min = Math.min(param.min || 0, 127);
            }
          });
        }
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Range slider thumb drag
    container.querySelectorAll('.om-range-thumb').forEach(function(thumb) {
      thumb.addEventListener('pointerdown', function(e) {
        e.preventDefault();
        thumb.setPointerCapture(e.pointerId);
        thumb.classList.add('dragging');

        var mi = parseInt(thumb.getAttribute('data-mapping'), 10);
        var ai = parseInt(thumb.getAttribute('data-add'), 10);
        var isLow = thumb.classList.contains('om-thumb-low');
        var track = thumb.closest('.om-range-track');
        if (!track) return;
        var parentRow = thumb.closest('.om-cc-row, .om-source-row');

        function onMove(ev) {
          if (!state.editDef || !state.editDef.mapping[mi]) return;
          var mapping = state.editDef.mapping[mi];
          var maxCC = is14bit(mapping) ? 16383 : 127;
          var rect = track.getBoundingClientRect();
          var pct = (ev.clientX - rect.left) / rect.width;
          pct = Math.max(0, Math.min(1, pct));
          var ccVal = Math.round(pct * maxCC);
          var range = sourceToRange(mapping, ai);
          var low = range.low, high = range.high;
          if (isLow) { low = Math.min(ccVal, high); } else { high = Math.max(ccVal, low); }
          rangeToSource(mapping, ai, low, high);
          state.dirty = true;

          var newRange = sourceToRange(mapping, ai);
          var lowPct = newRange.low / maxCC * 100;
          var highPct = newRange.high / maxCC * 100;
          var fill = track.querySelector('.om-range-fill');
          var thumbLow = track.querySelector('.om-thumb-low');
          var thumbHigh = track.querySelector('.om-thumb-high');
          if (fill) { fill.style.left = lowPct + '%'; fill.style.width = (highPct - lowPct) + '%'; }
          if (thumbLow) thumbLow.style.left = lowPct + '%';
          if (thumbHigh) thumbHigh.style.left = highPct + '%';
          if (parentRow) {
            var lowInput = parentRow.querySelector('.om-range-low');
            var highInput = parentRow.querySelector('.om-range-high');
            if (lowInput) lowInput.value = newRange.low;
            if (highInput) highInput.value = newRange.high;
          }
        }

        function onUp() {
          thumb.classList.remove('dragging');
          thumb.removeEventListener('pointermove', onMove);
          thumb.removeEventListener('pointerup', onUp);
          renderMacroBuilderSection();
        }

        thumb.addEventListener('pointermove', onMove);
        thumb.addEventListener('pointerup', onUp);
      });
    });

    // Add CC mapping for knob
    container.querySelectorAll('.mapping-add-cc-for-knob').forEach(function(select) {
      select.addEventListener('change', function() {
        if (!state.editDef) return;
        var ctrl = parseInt(select.value, 10);
        var srcIdx = parseInt(select.getAttribute('data-src-idx'), 10);
        if (isNaN(ctrl) || isNaN(srcIdx)) return;
        var addEntry = { src: srcIdx, mul: 1, div: 1 };

        // Auto-fill name + ui type for this knob based on the CC being mapped
        var machine = state.editDef.machine;
        if (machine) {
          var machineInfo = S.getMachineInfo(machine);
          if (machineInfo && machineInfo.parameters) {
            var ccParam = machineInfo.parameters.find(function(p) { return p.ctrl === ctrl; });
            if (ccParam) {
              var uiInfo = lookupUiType(machine, ccParam.id);
              var isNrpm = ccParam.type === 'nrpm';
              // Find the knob parameter and update its name, ui + curve
              state.editDef.groups.forEach(function(g) {
                (g.parameters || []).forEach(function(p) {
                  if (p.idx === srcIdx) {
                    // Auto-fill name if still generic (user can still rename later)
                    if (!p.name || p.name === 'New Knob' || /^CC\d+$/.test(p.name)) {
                      p.name = ccParam.name || ('CC' + ctrl);
                    }
                    p.ui = uiInfo.ui;
                    if (uiInfo.curve) { p.curve = uiInfo.curve; addEntry.curve = uiInfo.curve; }
                    else { delete p.curve; }
                    // Set 14-bit range for NRPN parameters
                    if (isNrpm && (p.max || 127) <= 127) {
                      p.max = 16383;
                      p.res = Math.round((p.res || 64) * 16383 / 127);
                      p.def = Math.round((p.def || 0) * 16383 / 127);
                    }
                  }
                });
              });
            }
          }
        }

        // Invariant: every mapping entry MUST carry an explicit "type" field
        // ("cc" or "nrpm") matching synthdefinitions.json for the same ctrl.
        // See tbd-pico-seq3/docs/architecture/macro-system.md — a missing type
        // silently defaults to CC on the Pico and regresses other machines
        // when the synthdef actually declares the ctrl as NRPM.
        var newMapping = { ctrl: ctrl, type: 'cc', start: 0, add: [addEntry] };
        if (machine) {
          var mi2 = S.getMachineInfo(machine);
          if (mi2 && mi2.parameters) {
            var cp = mi2.parameters.find(function(p) { return p.ctrl === ctrl; });
            if (cp && cp.type === 'nrpm') { newMapping.bits = 14; newMapping.type = 'nrpm'; }
          }
        }
        state.editDef.mapping.push(newMapping);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Add unmapped CC as constant
    var unmappedSelect = container.querySelector('.add-unmapped-cc-select');
    if (unmappedSelect) {
      unmappedSelect.addEventListener('change', function() {
        if (!state.editDef) return;
        var ctrl = parseInt(unmappedSelect.value, 10);
        if (isNaN(ctrl)) return;
        // Invariant: every mapping entry MUST carry an explicit "type" field
        // matching synthdefinitions.json. See the knob-to-CC branch above
        // for the full rationale.
        var unmappedType = 'cc';
        var unmappedBits;
        var unmappedMachine = state.editDef && state.editDef.machine;
        if (unmappedMachine) {
          var umi = S.getMachineInfo(unmappedMachine);
          if (umi && umi.parameters) {
            var ucp = umi.parameters.find(function(p) { return p.ctrl === ctrl; });
            if (ucp && ucp.type === 'nrpm') {
              unmappedType = 'nrpm';
              unmappedBits = 14;
            }
          }
        }
        var unmappedEntry = { ctrl: ctrl, type: unmappedType, start: 0, add: [] };
        if (unmappedBits) unmappedEntry.bits = unmappedBits;
        state.editDef.mapping.push(unmappedEntry);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    }

    // Remove output mapping
    container.querySelectorAll('.remove-mapping-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        if (!state.editDef) return;
        state.editDef.mapping.splice(mi, 1);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Add/remove source
    container.querySelectorAll('.mapping-add-src-select').forEach(function(select) {
      select.addEventListener('change', function() {
        var mi = parseInt(select.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var src = parseInt(select.value, 10);
        if (isNaN(src)) return;
        state.editDef.mapping[mi].add.push({ src: src, mul: 1, div: 1 });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });
    container.querySelectorAll('.mapping-remove-src-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        var ai = parseInt(btn.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        state.editDef.mapping[mi].add.splice(ai, 1);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Create preset and Save/delete preset handlers are now in setupSoundPresetsSectionEvents
  }

  // ─── Create 1:1 Mapping ──────────────────────────────────

  function createOneToOneMapping() {
    if (!state.editDef || !state.editDef.machine) {
      S.toast('Select a machine first', 'warning', 2000);
      return;
    }
    var machineInfo = S.getMachineInfo(state.editDef.machine);
    if (!machineInfo || !machineInfo.parameters || machineInfo.parameters.length === 0) {
      S.toast('Machine has no CC parameters', 'warning', 2000);
      return;
    }
    if (!confirm('This will replace all parameter groups and output mappings with a 1:1 mapping from all ' + machineInfo.parameters.length + ' CCs. Continue?')) {
      return;
    }

    var params = machineInfo.parameters;
    if (!state.editDef.id) { state.editDef.id = state.editDef.machine + '-allparams'; }
    if (!state.editDef.name) { state.editDef.name = (machineInfo.name || state.editDef.machine) + ' All params'; }

    var machine = state.editDef.machine;

    state.editDef.groups = [];
    var paramIdx = 0;
    var isRompler = machine === 'ro';
    for (var g = 0; g < 6 && paramIdx < params.length; g++) {
      var groupParams = [];
      for (var p = 0; p < 4 && paramIdx < params.length; p++) {
        var cc = params[paramIdx];
        var uiInfo = lookupUiType(machine, cc.id);
        var ccMax = (cc.type === 'nrpm') ? 16383 : 127;
        var paramObj = {
          idx: paramIdx,
          name: cc.name || ('CC' + cc.ctrl),
          def: cc.def || 0,
          min: 0, max: ccMax, res: Math.round(ccMax / 2), ui: uiInfo.ui,
        };
        if (uiInfo.curve) paramObj.curve = uiInfo.curve;
        groupParams.push(paramObj);
        paramIdx++;
      }
      var pageName = (isRompler && g === 0) ? 'SAMPLE' : 'Page ' + (g + 1);
      state.editDef.groups.push({ name: pageName, parameters: groupParams });
    }

    state.editDef.mapping = [];
    var allParams = [];
    state.editDef.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) { allParams.push(p); });
    });
    params.forEach(function(cc, i) {
      if (i < allParams.length) {
        var mapStart = 0;
        var mapMul = 1;
        var mapCurve = allParams[i].curve;
        if (isRompler) {
          // Attack/Decay need start:1 to avoid div-by-zero in envelope
          if (cc.id === 'attack' || cc.id === 'decay') mapStart = 1;
          // TSMode needs mul:64 for proper 0/1/2 integer mapping
          if (cc.id === 'tsmode') mapMul = 64;
        }
        var addEntry = { src: allParams[i].idx, mul: mapMul, div: 1 };
        if (mapCurve) addEntry.curve = mapCurve;
        var mapEntry = { ctrl: cc.ctrl, start: mapStart, add: [addEntry] };
        if (cc.type === 'nrpm') { mapEntry.bits = 14; mapEntry.type = 'nrpm'; }
        state.editDef.mapping.push(mapEntry);
      }
    });

    state.dirty = true;
    renderMacroBuilderSection();

    // Sync with performer for knob preview
    if (window.TBD.performer && window.TBD.performer.setMacroDef) {
      window.TBD.performer.setMacroDef(state.editDef);
    }

    S.toast('Created 1:1 mapping with ' + params.length + ' parameters', 'success', 2000);
  }

  // ─── Sound Preset CRUD ──────────────────────────────────

  function createSoundPresetForDef() {
    if (!state.editDef || !state.editDef.id) {
      S.toast('Save the definition first', 'warning', 2000);
      return;
    }

    var old = document.getElementById('create-preset-dialog');
    if (old) old.remove();

    var macroName = state.editDef.name || state.editDef.id;
    var defaultGroup = state.editDef.machine || 'User';
    var paramCount = 0;
    if (state.editDef.groups) {
      state.editDef.groups.forEach(function(g) { paramCount += (g.parameters || []).length; });
    }

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'create-preset-dialog';
    dialog.label = 'New Sound Preset';
    dialog.setAttribute('style', '--width:26rem;');

    var html = '';
    html += '<div style="display:flex;flex-direction:column;gap:0.75rem;">';
    html += '<sl-input id="new-preset-name" label="Preset Name" placeholder="e.g. Fat Bass" required autofocus></sl-input>';
    html += '<sl-input id="new-preset-group" label="Category / Group" value="' + S.esc(defaultGroup) + '" placeholder="e.g. User" help-text="Presets are grouped by this label in the sidebar"></sl-input>';
    html += '</div>';
    html += '<div style="margin-top:1rem;font-size:0.72rem;color:var(--sl-color-neutral-500);">';
    html += '<sl-icon name="info-circle" style="font-size:0.7rem;"></sl-icon> ';
    html += 'Creates a new preset with default values (' + paramCount + ' params) for the <strong>' + S.esc(macroName) + '</strong> macro.';
    html += '</div>';
    dialog.innerHTML = html;

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var createBtn = document.createElement('sl-button');
    createBtn.setAttribute('slot', 'footer');
    createBtn.setAttribute('variant', 'primary');
    createBtn.innerHTML = '<sl-icon name="plus-lg" slot="prefix"></sl-icon> Create Preset';
    createBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#new-preset-name');
      var groupInput = dialog.querySelector('#new-preset-group');
      var name = (nameInput.value || '').trim();
      var group = (groupInput.value || '').trim() || 'User';

      if (!name) {
        nameInput.setAttribute('help-text', 'Please enter a name');
        nameInput.focus();
        return;
      }

      var id = name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');

      // Prevent overwriting factory presets (unless Factory Edit Mode unlocked)
      var Fcheck = window.TBD.factory;
      var isFactoryId = Fcheck && Fcheck.isFactoryPreset(id);
      if (isFactoryId && !(Fcheck.isUnlocked && Fcheck.isUnlocked())) {
        nameInput.setAttribute('help-text', 'This name matches a factory preset \u2014 unlock Factory Edit Mode or choose a different name');
        nameInput.focus();
        return;
      }

      // Build a dense values array — fill any gaps with 0
      var maxIdx = -1;
      state.editDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) { if (p.idx > maxIdx) maxIdx = p.idx; });
      });
      var values = [];
      for (var vi = 0; vi <= maxIdx; vi++) values[vi] = 0;
      state.editDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) { values[p.idx] = p.def || 0; });
      });

      var preset = { id: id, name: name, group: group, macro: state.editDef.id, values: values };
      var jsonStr = JSON.stringify(preset, null, 2);
      var filePath = (isFactoryId ? 'factory/presets/' : 'presets/') + id + '.json';

      createBtn.setAttribute('loading', '');
      fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: jsonStr,
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        dialog.hide();
        S.toast('Created preset: ' + name, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderMacroBuilderSection();
      }).catch(function(err) {
        createBtn.removeAttribute('loading');
        S.toast('Create failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(createBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  function saveEditedPreset(presetId, container) {
    var F = window.TBD.factory;
    var isFactory = F && F.isFactoryPreset(presetId);
    if (isFactory && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory presets are read-only — unlock Factory Edit Mode first', 'warning', 3000);
      return;
    }
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    if (!preset) { S.toast('Preset not found', 'danger', 2000); return; }

    var nameInput = container.querySelector('.preset-name-input[data-preset-id="' + presetId + '"]');
    if (nameInput) preset.name = nameInput.value;
    var groupInput = container.querySelector('.preset-group-input[data-preset-id="' + presetId + '"]');
    if (groupInput) preset.group = groupInput.value;
    container.querySelectorAll('.preset-value-input[data-preset-id="' + presetId + '"]').forEach(function(input) {
      var vi = parseInt(input.getAttribute('data-value-idx'), 10);
      preset.values[vi] = parseInt(input.value, 10) || 0;
    });

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = (isFactory ? 'factory/presets/' : 'presets/') + presetId + '.json';

    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved preset: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderMacroBuilderSection();
    }).catch(function(err) {
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  function deleteSoundPreset(presetId) {
    var F = window.TBD.factory;
    if (F && F.isFactoryPreset(presetId) && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory presets cannot be deleted \u2014 unlock Factory Edit Mode first', 'warning', 3000);
      return;
    }
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    var displayName = preset ? preset.name : presetId;

    var old = document.getElementById('delete-preset-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'delete-preset-dialog';
    dialog.label = 'Delete Sound Preset';
    dialog.setAttribute('style', '--width:24rem;');

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0;">Are you sure you want to delete <strong>' + S.esc(displayName) + '</strong>?</p>'
      + '<p style="font-size:0.75rem;color:var(--sl-color-neutral-500);margin:0.5rem 0 0;">This action cannot be undone.</p>';

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var deleteBtn = document.createElement('sl-button');
    deleteBtn.setAttribute('slot', 'footer');
    deleteBtn.setAttribute('variant', 'danger');
    deleteBtn.innerHTML = '<sl-icon name="trash3" slot="prefix"></sl-icon> Delete';
    deleteBtn.addEventListener('click', function() {
      deleteBtn.setAttribute('loading', '');
      var Fdel = window.TBD.factory;
      var filePath = (Fdel && Fdel.isFactoryPreset(presetId) ? 'factory/presets/' : 'presets/') + presetId + '.json';
      apiPost('/api/v2/storage?action=manage', { action: 'deleteconfig', path: filePath })
      .then(function() {
        dialog.hide();
        S.toast('Deleted preset: ' + displayName, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderMacroBuilderSection();
      }).catch(function(err) {
        deleteBtn.removeAttribute('loading');
        S.toast('Delete failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(deleteBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  // ─── Delete Macro Definition ────────────────────────────────

  function deleteDefinition(defId) {
    var F = window.TBD.factory;
    if (F && F.isFactoryDefinition(defId) && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory definitions cannot be deleted — unlock Factory Edit Mode first', 'warning', 3000);
      return;
    }
    var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
    var displayName = def ? (def.name || def.id) : defId;

    // Check if any sound presets use this definition
    var dependentPresets = S.data.soundPresets.filter(function(p) { return p.macro === defId; });

    var old = document.getElementById('delete-def-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'delete-def-dialog';
    dialog.label = 'Delete Macro Definition';
    dialog.setAttribute('style', '--width:26rem;');

    var warningHtml = '';
    if (dependentPresets.length > 0) {
      warningHtml = '<div style="margin-top:0.75rem;padding:0.5rem 0.65rem;background:var(--sl-color-warning-100);border:1px solid var(--sl-color-warning-300);border-radius:4px;font-size:0.75rem;color:var(--sl-color-warning-700);">'
        + '<sl-icon name="exclamation-triangle" style="font-size:0.75rem;"></sl-icon> '
        + '<strong>' + dependentPresets.length + ' sound preset' + (dependentPresets.length > 1 ? 's' : '') + '</strong> use this macro definition and will become orphaned.'
        + '</div>';
    }

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0;">Are you sure you want to delete the macro definition <strong>' + S.esc(displayName) + '</strong>?</p>'
      + '<p style="font-size:0.75rem;color:var(--sl-color-neutral-500);margin:0.5rem 0 0;">This action cannot be undone.</p>'
      + warningHtml;

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var deleteBtn = document.createElement('sl-button');
    deleteBtn.setAttribute('slot', 'footer');
    deleteBtn.setAttribute('variant', 'danger');
    deleteBtn.innerHTML = '<sl-icon name="trash3" slot="prefix"></sl-icon> Delete';
    deleteBtn.addEventListener('click', function() {
      deleteBtn.setAttribute('loading', '');
      var Fdel = window.TBD.factory;
      var filePath = (Fdel && Fdel.isFactoryDefinition(defId) ? 'factory/macros/' : 'macros/') + defId + '.json';
      apiPost('/api/v2/storage?action=manage', { action: 'deleteconfig', path: filePath })
      .then(function() {
        dialog.hide();
        // If the deleted def was being edited, clear state
        if (state.selectedDefId === defId) {
          state.selectedDefId = null;
          state.editDef = null;
          state.dirty = false;
        }
        S.toast('Deleted macro: ' + displayName, 'success', 2000);
        // Reload firmware macro state, then refresh UI data
        return S.reloadFirmwareMacros(defId).then(function() {
          return S.reloadMacroData();
        });
      }).then(function() {
        renderDefinitionList();
        renderMacroBuilderSection();
        // Refresh performer preset browser and track info
        if (window.TBD.performer && window.TBD.performer.reload) {
          window.TBD.performer.reload();
        }
      }).catch(function(err) {
        deleteBtn.removeAttribute('loading');
        S.toast('Delete failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(deleteBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  // ─── Toolbar Actions (called by app.js) ──────────────────

  function saveDefinition() {
    if (!state.editDef) { S.toast('Nothing to save', 'warning', 2000); return; }
    if (!state.editDef.id) { S.toast('Definition ID is required', 'warning', 2000); return; }
    if (!state.editDef.machine) { S.toast('Select a machine for this definition', 'warning', 2000); return; }

    // Factory definitions: if unlocked, allow in-place save; otherwise prompt for clone
    var F = window.TBD.factory;
    if (F && F.isFactoryDefinition(state.editDef.id)) {
      if (F.isUnlocked && F.isUnlocked()) {
        // Unlocked — allow in-place save of factory definition
      } else {
        var newId = prompt('Factory definitions are read-only.\nEnter a new ID to save as a copy:', state.editDef.id + '-custom');
        if (!newId) return;
        newId = newId.trim().toLowerCase().replace(/[^a-z0-9_-]/g, '-').replace(/^-|-$/g, '');
        if (!newId) { S.toast('Invalid ID', 'warning', 2000); return; }
        if (F.isFactoryDefinition(newId)) { S.toast('That ID is also a factory definition', 'warning', 2000); return; }
        state.editDef.id = newId;
        state.selectedDefId = newId;
      }
    }

    var cleanDef = cleanDefinitionForSave(state.editDef);
    var jsonStr = JSON.stringify(cleanDef, null, 2);
    var isFactoryDef = F && F.isFactoryDefinition(state.editDef.id);
    var filePath = (isFactoryDef && F.isUnlocked && F.isUnlocked() ? 'factory/macros/' : 'macros/') + state.editDef.id + '.json';

    S.showLoading('Saving definition\u2026');
    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved: ' + state.editDef.name, 'success', 2000);
      state.dirty = false;
      // Reload firmware macro state, then refresh UI data
      return S.reloadFirmwareMacros(state.editDef.id).then(function() {
        return S.reloadMacroData();
      });
    }).then(function() {
      renderDefinitionList();
      // Sync performer with the saved definition
      if (window.TBD.performer && window.TBD.performer.setMacroDef) {
        window.TBD.performer.setMacroDef(state.editDef);
      }
      S.hideLoading();
    }).catch(function(err) {
      S.hideLoading();
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  function exportDefinition() {
    if (!state.editDef) { S.toast('Nothing to export', 'warning', 2000); return; }
    var cleanDef = cleanDefinitionForSave(state.editDef);
    var blob = new Blob([JSON.stringify(cleanDef, null, 2)], { type: 'application/json' });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = (state.editDef.id || 'definition') + '.json';
    a.click();
    URL.revokeObjectURL(a.href);
    S.toast('Exported: ' + (state.editDef.name || state.editDef.id), 'success', 2000);
  }

  function importDefinitionFile() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', function() {
      if (!input.files.length) return;
      var reader = new FileReader();
      reader.onload = function() {
        try {
          var data = JSON.parse(reader.result);
          if (data.groups && data.id) {
            // Validate machine matches current track (if a track is selected)
            if (state.editDef && state.editDef.machine && data.machine && data.machine !== state.editDef.machine) {
              if (!confirm('This definition is for machine "' + data.machine + '" but the current track uses "' + state.editDef.machine + '". Import anyway?')) return;
            }
            state.editDef = data;
            state.selectedDefId = data.id || null;
            ensureGroupStructure(state.editDef);
            reindexParameters(state.editDef);
            state.dirty = true;
            renderMacroBuilderSection();
            // Sync performer with the imported definition
            if (window.TBD.performer && window.TBD.performer.setMacroDef) {
              window.TBD.performer.setMacroDef(state.editDef);
            }
            S.toast('Imported definition: ' + (data.name || data.id || 'unknown'), 'success', 2000);
          } else if (data.id && data.macro && data.values) {
            var Fimp = window.TBD.factory;
            var isFactoryId = Fimp && Fimp.isFactoryPreset && Fimp.isFactoryPreset(data.id);
            var filePath = (isFactoryId && Fimp.isUnlocked && Fimp.isUnlocked() ? 'factory/presets/' : 'presets/') + data.id + '.json';
            fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
              method: 'POST',
              headers: { 'Content-Type': 'application/json' },
              body: JSON.stringify(data, null, 2),
            }).then(function(r) {
              if (!r.ok) throw new Error('HTTP ' + r.status);
              S.toast('Imported preset: ' + data.name, 'success', 2000);
              return S.reloadMacroData();
            }).then(function() {
              renderMacroBuilderSection();
            }).catch(function(err) {
              S.toast('Import failed: ' + err.message, 'danger', 3000);
            });
          } else {
            S.toast('Unrecognized JSON format', 'warning', 3000);
          }
        } catch (err) {
          S.toast('Invalid JSON: ' + err.message, 'danger', 3000);
        }
      };
      reader.readAsText(input.files[0]);
    });
    input.click();
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    S.onTrackChange(function(idx, track) {
      onTrackSelected(idx, track);
    });

    setupDefinitionListEvents();

    // Re-render when factory lock state changes (delete buttons, edit permissions)
    window.addEventListener('tbd-factory-lock-changed', function() {
      renderDefinitionList();
      if (state.editDef) renderMacroBuilderSection();
    });

    if (S.data.activeTrack >= 0) {
      var track = S.data.tracks.find(function(t) { return t.index === S.data.activeTrack; });
      if (track) onTrackSelected(S.data.activeTrack, track);
    } else if (S.data.tracks.length > 0) {
      S.selectTrack(S.data.tracks[0].index);
    }

    state.initialized = true;
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.designer = {
    init: init,
    state: state,
    onMachineChanged: onMachineChanged,
    saveDefinition: saveDefinition,
    deleteDefinition: deleteDefinition,
    exportDefinition: exportDefinition,
    importDefinitionFile: importDefinitionFile,
    selectMacroDefinition: selectMacroDefinition,
    reload: function() {
      if (state.activeTrack >= 0) {
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) onTrackSelected(state.activeTrack, track);
      }
    },
  };

})();

// ── track-defaults.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Track Default Presets Editor (Right Panel)
//
// Renders inside #kit-panel as a right-panel editor mode.
// Toggle between File Viewer (JSON) and Editor via header buttons.
// Hierarchy mirrors the main Preset & Macro Manager UI:
//   Track → Machine → Preset (grouped by Macro definition)
//
// Machine names come from S.getMachineInfo() (synthdefinitions.json).
// Machine list uses S.getTrackMachines() which filters out noX empties.
// Presets are grouped into <optgroup>s by their macro definition name,
// so the user sees e.g. "Phat Punch" / "Synth Kick — All knobs" sections.
//
// Data source:  factory/ or user/trackdefaults/
// API:          GET  /api/v2/macros?action=get_trackdefaults&file=<name>
//               POST /api/v2/macros?action=save_trackdefaults
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ─────────────────────────────────────────────────
  var currentFile = null;     // { path, name } of the file being edited

  var trackDefaults = null;   // parsed trackdefaults.json
  var dirty = false;
  var facetedData = null;     // per-track: [ { machine, name, macros: [{id, name, presets}] } ]
  var kitNames = [];          // kit names from sample_rom.json via samples API
  var kitFiles = [];          // kit filenames from smp_banks (e.g. "def_smp.json")
  var kitMeta = [];           // per-kit bank metadata [{banks: [{name, color}]}]
  var activeKitIndex = 0;     // index of the currently active kit in PSRAM
  var activeKitEntries = [];  // sample entries for the active kit
  var activeTemplateName = 'default'; // name of the boot-default template (from P4)

  var SLICES_PER_BANK = 32;
  var DEFAULT_BANKS = [
    'KICK', 'SNARE', 'HIHAT CL', 'HIHAT OP',
    'CLAP', 'RIM', 'PERC', 'OTHER'
  ];

  // ─── Helpers ───────────────────────────────────────────────

  /**
   * Get display name for a machine ID using S.getMachineInfo()
   * (same source as the main UI's MACHINE: dropdown).
   */
  function getMachineName(machineId) {
    var info = S.getMachineInfo(machineId);
    return info ? info.name : machineId;
  }

  /**
   * Build faceted data for every track.
   * Uses S.getTrackMachines() to get the same machine list as the main UI
   * (filters out nodrum/nosynth/nofx).
   *
   * Returns { trackIndex: [ { machine, name, macros: [ { id, name, presets } ] } ] }
   */
  function buildFacetedData() {
    var tracks = S.data.tracks || [];
    var allDefs = S.data.macroDefs || [];
    var allPresets = S.data.soundPresets || [];

    var result = {};

    tracks.forEach(function(track) {
      // Use S.getTrackMachines() — same filter as the main UI MACHINE: dropdown
      var machines = S.getTrackMachines(track);
      var facets = [];

      machines.forEach(function(machineId) {
        // Find all macro definitions for this machine
        var defs = allDefs.filter(function(d) { return d.machine === machineId; });
        if (defs.length === 0) return;

        var macros = [];
        defs.forEach(function(def) {
          // Find all sound presets that use this macrodefinition
          var presets = allPresets.filter(function(p) { return p.macro === def.id; });
          if (presets.length === 0) return;
          presets.sort(function(a, b) {
            var na = (a.name || a.id).toLowerCase();
            var nb = (b.name || b.id).toLowerCase();
            return na < nb ? -1 : na > nb ? 1 : 0;
          });
          macros.push({ id: def.id, name: def.name || def.id, presets: presets });
        });

        // Only include machine if it has at least one preset
        if (macros.length > 0) {
          facets.push({
            machine: machineId,
            name: getMachineName(machineId),
            macros: macros
          });
        }
      });

      result[track.index] = facets;
    });

    return result;
  }

  /**
   * Given a preset ID, find which machine it belongs to within a track's facets.
   */
  function findMachineForPreset(presetId, facets) {
    for (var i = 0; i < facets.length; i++) {
      for (var j = 0; j < facets[i].macros.length; j++) {
        for (var k = 0; k < facets[i].macros[j].presets.length; k++) {
          if (facets[i].macros[j].presets[k].id === presetId) {
            return facets[i].machine;
          }
        }
      }
    }
    return '';
  }

  /**
   * Get the current default preset ID for a track from the loaded defaults.
   */
  function getDefaultPreset(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return '';
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry ? (entry.preset || '') : '';
  }

  /**
   * Get the saved sampleSlice for a track (rompler tracks only).
   */
  function getDefaultSlice(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return 0;
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry && typeof entry.sampleSlice === 'number' ? entry.sampleSlice : 0;
  }

  /**
   * Get the saved sampleBank (group index) for a track (rompler tracks only).
   */
  function getDefaultBank(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return 0;
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry && typeof entry.sampleBank === 'number' ? entry.sampleBank : 0;
  }

  /**
   * Get the number of populated banks for a kit index.
   * For the active kit, count from actual entries. Otherwise use metadata or default 8.
   */
  function getBankCountForKit(kitIndex) {
    if (kitIndex === activeKitIndex && activeKitEntries.length > 0) {
      return Math.ceil(activeKitEntries.length / SLICES_PER_BANK);
    }
    if (kitMeta && kitMeta[kitIndex] &&
        kitMeta[kitIndex].banks && kitMeta[kitIndex].banks.length > 0) {
      return kitMeta[kitIndex].banks.length;
    }
    return DEFAULT_BANKS.length;
  }

  /**
   * Get the number of valid slices for a given bank in a kit.
   * For the active kit, count actual entries. Otherwise return max 32.
   */
  function getSliceCountForBank(kitIndex, bankIndex) {
    if (kitIndex === activeKitIndex && activeKitEntries.length > 0) {
      var bankStart = bankIndex * SLICES_PER_BANK;
      var bankEnd = Math.min(bankStart + SLICES_PER_BANK, activeKitEntries.length);
      if (bankStart >= activeKitEntries.length) return 0;
      return bankEnd - bankStart;
    }
    return SLICES_PER_BANK;
  }

  /**
   * Get bank/group names for the currently selected kit index.
   * Uses smp_bank_meta from the samples API, with fallback to DEFAULT_BANKS.
   * Only returns names for banks that actually have entries.
   */
  function getBankNamesForKit(kitIndex) {
    var bankCount = getBankCountForKit(kitIndex);
    if (kitMeta && kitMeta[kitIndex] &&
        kitMeta[kitIndex].banks && kitMeta[kitIndex].banks.length > 0) {
      return kitMeta[kitIndex].banks.slice(0, bankCount).map(function(b) { return b.name; });
    }
    return DEFAULT_BANKS.slice(0, bankCount);
  }

  /**
   * Get display name for a slice: show sample filename if available, otherwise just the index.
   */
  function getSliceName(kitIndex, bankIndex, sliceIndex) {
    if (kitIndex === activeKitIndex && activeKitEntries.length > 0) {
      var entryIdx = bankIndex * SLICES_PER_BANK + sliceIndex;
      if (entryIdx < activeKitEntries.length) {
        var entry = activeKitEntries[entryIdx];
        if (entry && entry.filename) {
          return sliceIndex + ' — ' + entry.filename;
        }
      }
    }
    return String(sliceIndex);
  }

  /**
   * Rebuild the slice dropdown for a specific rompler track based on selected bank.
   */
  function rebuildSliceDropdown(trackIdx, kitIndex, bankIndex, selectedSlice) {
    var sliceCell = document.querySelector('.td-col-slice.td-rompler-cell[data-track="' + trackIdx + '"]');
    if (!sliceCell) return;
    var sliceCount = getSliceCountForBank(kitIndex, bankIndex);
    var html = '<select class="td-select td-slice-select" data-track="' + trackIdx + '">';
    for (var sl = 0; sl < sliceCount; sl++) {
      var slSel = (sl === selectedSlice) ? ' selected' : '';
      var sliceName = getSliceName(kitIndex, bankIndex, sl);
      html += '<option value="' + sl + '"' + slSel + '>' + S.esc(sliceName) + '</option>';
    }
    html += '</select>';
    sliceCell.innerHTML = html;
    sliceCell.querySelector('.td-slice-select').addEventListener('change', function() {
      dirty = true;
    });
  }

  /**
   * Rebuild all rompler Bank dropdowns when Kit changes.
   */
  function rebuildBankDropdowns() {
    var kitSel = document.getElementById('td-global-kit');
    var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
    var bankNames = getBankNamesForKit(kitIdx);
    var bankCount = bankNames.length;
    var selects = document.querySelectorAll('.td-bank-select');
    selects.forEach(function(sel) {
      var current = parseInt(sel.value, 10) || 0;
      if (current >= bankCount) current = 0;
      var html = '';
      bankNames.forEach(function(name, i) {
        var selected = (i === current) ? ' selected' : '';
        html += '<option value="' + i + '"' + selected + '>' + S.esc(name) + '</option>';
      });
      sel.innerHTML = html;
      // Also rebuild the slice dropdown for this track
      var trackIdx = parseInt(sel.getAttribute('data-track'), 10);
      var sliceSel = document.querySelector('.td-slice-select[data-track="' + trackIdx + '"]');
      var currentSlice = sliceSel ? parseInt(sliceSel.value, 10) || 0 : 0;
      rebuildSliceDropdown(trackIdx, kitIdx, current, currentSlice);
    });
  }

  /**
   * Check if a track supports the rompler machine (has 'ro' in machines list).
   */
  function trackHasRompler(track) {
    return (track.machines || []).indexOf('ro') !== -1;
  }

  // ─── API ───────────────────────────────────────────────────

  /**
   * Fetch kit names and per-kit bank metadata from the samples API.
   */
  function loadKitData() {
    return S.queuedFetch('/storage')
      .then(function(data) {
        if (data && data.kits && data.kits.smp_bank_names) {
          kitNames = data.kits.smp_bank_names;
        } else {
          kitNames = [];
        }
        if (data && data.kits && data.kits.smp_banks) {
          kitFiles = data.kits.smp_banks;
        } else {
          kitFiles = [];
        }
        if (data && data.kits && data.kits.smp_bank_meta) {
          kitMeta = data.kits.smp_bank_meta;
        } else {
          kitMeta = [];
        }
        if (data && data.kits && typeof data.kits.active_smp_bank === 'number') {
          activeKitIndex = data.kits.active_smp_bank;
        } else {
          activeKitIndex = 0;
        }
        if (data && data.active_kit_entries && Array.isArray(data.active_kit_entries)) {
          activeKitEntries = data.active_kit_entries;
        } else {
          activeKitEntries = [];
        }
        return kitNames;
      })
      .catch(function() {
        kitNames = [];
        kitFiles = [];
        kitMeta = [];
        activeKitEntries = [];
        return kitNames;
      });
  }

  function loadTrackDefaults() {
    var fileParam = currentFile && currentFile.name ? '&file=' + encodeURIComponent(currentFile.name) : '';
    return loadKitData().then(function() {
      return S.queuedFetch('/macros?action=get_trackdefaults' + fileParam);
    }).then(function(data) {
        trackDefaults = data && data.tracks ? data : { tracks: [] };
        return trackDefaults;
      })
      .catch(function(err) {
        console.warn('[TrackDefaults] Load failed, using empty defaults:', err);
        trackDefaults = { tracks: [] };
        return trackDefaults;
      });
  }

  function isFactoryFile() {
    return currentFile && /^factory\//i.test(currentFile.path || '');
  }

  function updateFactoryBadge() {
    var badge = document.getElementById('td-badge-factory');
    var factory = isFactoryFile();
    if (badge) badge.style.display = factory ? '' : 'none';
  }

  /**
   * Get the template name (without .json) for the current file.
   */
  function currentTemplateName() {
    if (!currentFile || !currentFile.name) return 'default';
    return currentFile.name.replace(/\.json$/i, '');
  }

  /**
   * Fetch which template is the active boot default from the P4.
   */
  function fetchActiveTemplate() {
    return S.queuedFetch('/macros?action=get_active_trackdefault')
      .then(function(data) {
        if (data && data.name) activeTemplateName = data.name;
        updateActiveBadge();
      })
      .catch(function() { /* ignore — badge stays as-is */ });
  }

  /**
   * Update the "boot default" badge and "Set as Boot Default" button.
   */
  function updateActiveBadge() {
    var badge = document.getElementById('td-badge-active');
    var btn = document.getElementById('td-setactive-btn');
    var isActive = (currentTemplateName() === activeTemplateName);
    if (badge) badge.style.display = isActive ? '' : 'none';
    if (btn) {
      btn.disabled = isActive;
      btn.setAttribute('name', isActive ? 'star-fill' : 'star');
      btn.setAttribute('data-active', isActive ? 'true' : 'false');
      btn.title = isActive ? 'Boot Default' : 'Set as Boot Default';
      btn.label = isActive ? 'Boot Default' : 'Set as Boot Default';
    }
  }

  /**
   * Set the current template as the boot default via REST API.
   */
  function setAsBootDefault() {
    var name = currentTemplateName();
    S.queuedPost('/macros?action=set_active_trackdefault', { name: name }, S.API_MUTATION_TIMEOUT_MS)
      .then(function(resp) {
        if (resp && resp.ok) {
          activeTemplateName = name;
          updateActiveBadge();
          S.toast('Boot default set to "' + name + '"', 'success', 3000);
        } else {
          S.toast('Failed to set boot default', 'danger', 4000);
        }
      })
      .catch(function(err) {
        S.toast('Failed to set boot default: ' + err.message, 'danger', 4000);
      });
  }

  function saveTrackDefaults(data, fileName) {
    var file = fileName || (currentFile ? currentFile.name : 'default.json');
    return S.queuedPost('/macros?action=save_trackdefaults&file=' + encodeURIComponent(file), data, S.API_MUTATION_TIMEOUT_MS)
      .then(function(resp) {
        if (resp && resp.ok) {
          S.toast('Track setup saved', 'success', 3000);
          dirty = false;
          updateFactoryBadge();
        } else {
          S.toast('Save failed', 'danger', 4000);
        }
        return resp;
      })
      .catch(function(err) {
        S.toast('Save failed: ' + err.message, 'danger', 4000);
        throw err;
      });
  }

  // ─── Save As Dialog (Shoelace) ───────────────────────────

  function saveAsDialog() {
    var old = document.getElementById('td-saveas-dialog');
    if (old) old.remove();

    var srcName = currentFile ? currentFile.name.replace(/\.json$/i, '') : 'default';
    var defaultName = srcName + '-custom';

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'td-saveas-dialog';
    dialog.label = 'Save Track Setup As…';
    dialog.setAttribute('style', '--width:26rem;');

    // Add footer gap between Cancel and Save buttons
    var footerStyle = document.createElement('style');
    footerStyle.textContent = '#td-saveas-dialog::part(footer){display:flex;gap:0.5rem;justify-content:flex-end;}';

    var html = '';
    html += '<div style="font-size:0.8rem;color:var(--sl-color-neutral-500);margin-bottom:0.75rem;">';
    html += '<sl-icon name="info-circle" style="font-size:0.7rem;"></sl-icon> ';
    html += 'Saves a copy to the user folder. The original factory template is not modified.';
    html += '</div>';
    html += '<sl-input id="td-saveas-name" label="Template Name" value="' + S.esc(defaultName) + '" placeholder="e.g. my-setup" required autofocus></sl-input>';
    html += '<div id="td-saveas-hint" style="font-size:0.72rem;color:var(--sl-color-neutral-400);margin-top:0.35rem;"></div>';

    dialog.innerHTML = html;

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var saveBtn = document.createElement('sl-button');
    saveBtn.setAttribute('slot', 'footer');
    saveBtn.setAttribute('variant', 'primary');
    saveBtn.innerHTML = '<sl-icon name="floppy" slot="prefix"></sl-icon> Save Copy';

    saveBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#td-saveas-name');
      var hint = dialog.querySelector('#td-saveas-hint');
      var raw = (nameInput.value || '').trim();

      if (!raw) {
        hint.textContent = 'Please enter a name';
        hint.style.color = 'var(--sl-color-danger-600)';
        nameInput.focus();
        return;
      }

      var safeName = raw.toLowerCase().replace(/[^a-z0-9_-]+/g, '-').replace(/^-|-$/g, '');
      if (!safeName) {
        hint.textContent = 'Name must contain at least one letter or digit';
        hint.style.color = 'var(--sl-color-danger-600)';
        nameInput.focus();
        return;
      }

      var fileName = safeName + '.json';
      var data = collectFromUI();

      saveBtn.setAttribute('loading', '');
      saveTrackDefaults(data, fileName)
        .then(function() {
          dialog.hide();
          // Switch currentFile to the new user copy
          currentFile = { path: 'user/trackdefaults', name: fileName };
          var nameEl = document.getElementById('td-file-name');
          if (nameEl) nameEl.textContent = fileName;
          dirty = false;
          updateFactoryBadge();
          // Refresh the file browser to show the new file
          if (window.TBD.sampleManager && window.TBD.sampleManager.refreshCurrentDir) {
            window.TBD.sampleManager.refreshCurrentDir();
          }
        })
        .catch(function() {
          saveBtn.removeAttribute('loading');
        });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(saveBtn);
    document.body.appendChild(dialog);
    document.head.appendChild(footerStyle);

    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); footerStyle.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  // ─── Preset dropdown builder ───────────────────────────────

  /**
   * Rebuild the preset <select> options for a given track row
   * when the machine dropdown changes.
   * Presets are grouped by their macro definition name.
   */
  function rebuildPresetDropdown(trackIdx, machineId, currentPresetId) {
    var presetSel = document.querySelector('.td-preset-select[data-track="' + trackIdx + '"]');
    if (!presetSel) return;

    var facets = facetedData[trackIdx] || [];
    var matchingFacet = null;
    for (var i = 0; i < facets.length; i++) {
      if (facets[i].machine === machineId) { matchingFacet = facets[i]; break; }
    }

    var html = '<option value="">(auto — first available)</option>';

    if (matchingFacet) {
      var macros = matchingFacet.macros;

      macros.forEach(function(macro) {
        // Clean up macro definition name for the optgroup label
        var label = (macro.name || macro.id);
        // If it ends with "All param(s)", make it clearer
        if (/All\s*param/i.test(label)) {
          label = label.replace(/\s*All\s*param(s)?\s*$/i, '') + ' — All knobs';
        }

        // Always show optgroup so user can see which macro def each preset belongs to
        html += '<optgroup label="' + S.esc(label) + '">';
        macro.presets.forEach(function(p) {
          var sel = p.id === currentPresetId ? ' selected' : '';
          var pName = p.name || p.id;
          html += '<option value="' + S.esc(p.id) + '"' + sel + '>';
          html += S.esc(pName) + ' (' + S.esc(p.id) + ') \u2014 Macro: ' + S.esc(macro.id);
          html += '</option>';
        });
        html += '</optgroup>';
      });
    }

    presetSel.innerHTML = html;
    presetSel.disabled = !machineId;

    // If the current saved preset wasn't found in the new machine, reset
    if (currentPresetId && presetSel.value !== currentPresetId) {
      presetSel.value = '';
    }

  }

  // ─── Rendering ─────────────────────────────────────────────

  /**
   * Toggle Bank & Slice cells when machine changes to/from Rompler.
   */
  function updateRomplerCells(trackIdx, machineId) {
    var bankCell = document.querySelector('.td-col-bank.td-rompler-cell[data-track="' + trackIdx + '"]');
    var sliceCell = document.querySelector('.td-col-slice.td-rompler-cell[data-track="' + trackIdx + '"]');
    if (!bankCell || !sliceCell) return;

    if (machineId === 'ro') {
      // Build bank dropdown
      var kitSel = document.getElementById('td-global-kit');
      var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
      var bankNames = getBankNamesForKit(kitIdx);
      var bankHtml = '<select class="td-select td-bank-select" data-track="' + trackIdx + '">';
      bankNames.forEach(function(name, i) {
        bankHtml += '<option value="' + i + '">' + S.esc(name) + '</option>';
      });
      bankHtml += '</select>';
      bankCell.innerHTML = bankHtml;

      // Build slice dropdown for bank 0
      rebuildSliceDropdown(trackIdx, kitIdx, 0, 0);

      // Attach change listeners to new selects
      bankCell.querySelector('.td-bank-select').addEventListener('change', function() {
        var newBank = parseInt(this.value, 10);
        rebuildSliceDropdown(trackIdx, kitIdx, newBank, 0);
        dirty = true;
      });
      sliceCell.querySelector('.td-slice-select').addEventListener('change', function() { dirty = true; });
    } else {
      bankCell.innerHTML = '<span class="td-na">—</span>';
      sliceCell.innerHTML = '<span class="td-na">—</span>';
    }
  }

  function renderOverlayContent() {
    var body = document.getElementById('td-editor-body');
    if (!body) return;

    facetedData = buildFacetedData();
    var tracks = S.data.tracks || [];

    var html = '';
    html += '<p class="td-intro">Configure which preset each track loads on boot. ';
    html += 'Pick a <strong>machine</strong> first, then choose a <strong>preset</strong>. ';
    html += 'Changes take effect on next power-up.</p>';;

    // ─── Global kit selector ─────────────────────────────────
    if (kitNames.length > 0) {
      var savedKitFile = (trackDefaults && typeof trackDefaults.kit === 'string')
        ? trackDefaults.kit : (kitFiles.length > 0 ? kitFiles[0] : '');
      var savedKit = kitFiles.indexOf(savedKitFile);
      if (savedKit < 0) savedKit = 0;
      html += '<div class="td-sample-bank-section">';
      html += '<label><strong>Kit (PSRAM)</strong> ';
      html += '<select class="td-select" id="td-global-kit">';
      kitNames.forEach(function(name, i) {
        var sel = (i === savedKit) ? ' selected' : '';
        html += '<option value="' + i + '"' + sel + '>' + S.esc(name) + '</option>';
      });
      html += '</select></label>';
      html += '<span class="td-hint"> All rompler tracks share this kit. ';
      html += 'Switching reloads PSRAM from SD card at boot.</span>';
      html += '</div>';
    }

    html += '<div class="td-table">';
    html += '<div class="td-row td-header">';
    html += '<span class="td-col-idx">#</span>';
    html += '<span class="td-col-name">Track</span>';
    html += '<span class="td-col-type">Type</span>';
    html += '<span class="td-col-engine">Machine</span>';
    html += '<span class="td-col-preset">Preset</span>';
    html += '<span class="td-col-bank">Bank</span>';
    html += '<span class="td-col-slice">Slice</span>';
    html += '</div>';

    tracks.forEach(function(track) {
      var idx = track.index;
      var currentPreset = getDefaultPreset(idx);
      var facets = facetedData[idx] || [];

      // Determine current machine from the saved preset
      var currentMachine = currentPreset ? findMachineForPreset(currentPreset, facets) : '';

      // Track type badge
      var typeClass = 'td-type-badge';
      var isMaster = track.index === 18 || (track.machines && track.machines.indexOf('fxmaster') >= 0);
      if (isMaster) typeClass += ' td-type-master';
      else if (track.type === 'drum')  typeClass += ' td-type-drum';
      else if (track.type === 'synth') typeClass += ' td-type-synth';
      else if (track.type === 'fx')    typeClass += ' td-type-fx';

      html += '<div class="td-row" data-track="' + idx + '">';
      html += '<span class="td-col-idx">' + String(idx + 1).padStart(2, '0') + '</span>';
      html += '<span class="td-col-name">' + S.esc(track.name) + '</span>';
      html += '<span class="td-col-type"><span class="' + typeClass + '">' + S.esc(isMaster ? 'master' : track.type) + '</span></span>';

      // Machine dropdown — same options as the main UI MACHINE: dropdown
      html += '<span class="td-col-engine">';
      html += '<select class="td-select td-machine-select" data-track="' + idx + '">';
      html += '<option value="">(auto)</option>';
      facets.forEach(function(f) {
        var sel = f.machine === currentMachine ? ' selected' : '';
        html += '<option value="' + S.esc(f.machine) + '"' + sel + '>' + S.esc(f.name) + '</option>';
      });
      html += '</select>';
      html += '</span>';

      // Preset dropdown (populated dynamically based on machine selection)
      html += '<span class="td-col-preset">';
      html += '<select class="td-select td-preset-select" data-track="' + idx + '"';
      if (!currentMachine) html += ' disabled';
      html += '>';
      html += '<option value="">(auto — first available)</option>';
      html += '</select>';
      html += '</span>';

      // Bank + Slice selectors — only shown when selected machine is Rompler
      var isSelectedRompler = (currentMachine === 'ro');
      var savedBank = getDefaultBank(idx);
      var savedSlice = getDefaultSlice(idx);
      var savedKitFile2 = (trackDefaults && typeof trackDefaults.kit === 'string')
        ? trackDefaults.kit : (kitFiles.length > 0 ? kitFiles[0] : '');
      var kitIdx = kitFiles.indexOf(savedKitFile2);
      if (kitIdx < 0) kitIdx = 0;
      var bankNames = getBankNamesForKit(kitIdx);

      html += '<span class="td-col-bank td-rompler-cell" data-track="' + idx + '">';
      if (isSelectedRompler) {
        html += '<select class="td-select td-bank-select" data-track="' + idx + '">';
        bankNames.forEach(function(name, bi) {
          var bSel = (bi === savedBank) ? ' selected' : '';
          html += '<option value="' + bi + '"' + bSel + '>' + S.esc(name) + '</option>';
        });
        html += '</select>';
      } else {
        html += '<span class="td-na">—</span>';
      }
      html += '</span>';

      html += '<span class="td-col-slice td-rompler-cell" data-track="' + idx + '">';
      if (isSelectedRompler) {
        var sliceCount = getSliceCountForBank(kitIdx, savedBank);
        html += '<select class="td-select td-slice-select" data-track="' + idx + '">';
        for (var sl = 0; sl < sliceCount; sl++) {
          var slSel = (sl === savedSlice) ? ' selected' : '';
          var sliceName = getSliceName(kitIdx, savedBank, sl);
          html += '<option value="' + sl + '"' + slSel + '>' + sliceName + '</option>';
        }
        html += '</select>';
      } else {
        html += '<span class="td-na">—</span>';
      }
      html += '</span>';

      html += '</div>';
    });

    html += '</div>'; // .td-table

    body.innerHTML = html;

    // Populate preset dropdowns for tracks that have a saved machine
    tracks.forEach(function(track) {
      var currentPreset = getDefaultPreset(track.index);
      var facets = facetedData[track.index] || [];
      var currentMachine = currentPreset ? findMachineForPreset(currentPreset, facets) : '';
      if (currentMachine) {
        rebuildPresetDropdown(track.index, currentMachine, currentPreset);
      }
    });

    // Attach machine change listeners
    body.querySelectorAll('.td-machine-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var idx = parseInt(sel.getAttribute('data-track'), 10);
        var machineId = sel.value;
        rebuildPresetDropdown(idx, machineId, '');

        // Auto-select first available preset when machine changes
        var presetSel = document.querySelector('.td-preset-select[data-track="' + idx + '"]');
        if (presetSel && presetSel.options.length > 1) {
          presetSel.selectedIndex = 1; // first real preset (index 0 is "(auto)")
        }
        // Show/hide Bank & Slice cells based on whether machine is Rompler
        updateRomplerCells(idx, machineId);

        dirty = true;
       
      });
    });

    // Attach preset change listeners
    body.querySelectorAll('.td-preset-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
       
      });
    });

    // Attach slice change listeners
    body.querySelectorAll('.td-slice-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
       
      });
    });

    // Attach bank (group) change listeners — also rebuilds slice dropdown
    body.querySelectorAll('.td-bank-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var trackIdx = parseInt(sel.getAttribute('data-track'), 10);
        var kitSel2 = document.getElementById('td-global-kit');
        var kitIdx2 = kitSel2 ? parseInt(kitSel2.value, 10) : 0;
        var newBank = parseInt(sel.value, 10);
        rebuildSliceDropdown(trackIdx, kitIdx2, newBank, 0);
        dirty = true;
       
      });
    });

    // Attach global kit change listener — also refreshes bank dropdowns
    var kitSel = document.getElementById('td-global-kit');
    if (kitSel) {
      kitSel.addEventListener('change', function() {
        rebuildBankDropdowns();
        dirty = true;
       
      });
    }

    dirty = false;
  }

  // ─── Collect & Save ────────────────────────────────────────

  function collectFromUI() {
    // Read global kit selection
    var kitSel = document.getElementById('td-global-kit');
    var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
    var kitFile = (kitFiles.length > kitIdx) ? kitFiles[kitIdx] : (kitFiles[0] || 'def_smp.json');

    var result = {
      _comment: 'Default preset per track, loaded by the Pico via SPI command 0xA5.',
      _comment2: 'Preset IDs = filenames (without .json) from presets/.',
      _comment3: 'Omit a track entry to let the Pico use the first available preset.',
      _comment4: 'The kit field sets which kit file to activate in PSRAM (matched by filename).',
      _comment5: 'NOTE: all romplers share the same PSRAM kit — only one kit is active at a time.',
      kit: kitFile,
      tracks: []
    };

    var tracks = S.data.tracks || [];
    var presetSelects = document.querySelectorAll('#td-editor-body .td-preset-select');

    presetSelects.forEach(function(sel) {
      var idx = parseInt(sel.getAttribute('data-track'), 10);
      var presetId = sel.value;
      if (presetId) {
        var track = tracks.find(function(t) { return t.index === idx; });
        var trackName = track ? track.name : ('Track ' + idx);

        // Include machine name in the comment for readability
        var machineSel = document.querySelector('.td-machine-select[data-track="' + idx + '"]');
        var machineName = machineSel ? machineSel.options[machineSel.selectedIndex].text : '';

        var entry = {
          index: idx,
          preset: presetId,
          _name: trackName + ' — ' + machineName + ' — ' + presetId
        };

        // Add rompler-specific fields only when machine is set to Rompler
        var machineVal = machineSel ? machineSel.value : '';
        if (machineVal === 'ro') {
          var bankGroupSel = document.querySelector('.td-bank-select[data-track="' + idx + '"]');
          entry.sampleBank = bankGroupSel ? parseInt(bankGroupSel.value, 10) : 0;
          var sliceSel = document.querySelector('.td-slice-select[data-track="' + idx + '"]');
          entry.sampleSlice = sliceSel ? parseInt(sliceSel.value, 10) : 0;
        }

        result.tracks.push(entry);
      }
    });

    return result;
  }

  // ─── Panel switching ────────────────────────────────────────

  function showTDPanel() {
    var panel = document.getElementById('kit-panel');
    panel.classList.remove('viewer-active', 'pv-active');
    panel.classList.add('td-editor-active');
    // Update filename in the sub-header tab
    var nameEl = document.getElementById('td-file-name');
    if (nameEl) nameEl.textContent = currentFile ? currentFile.name : 'default.json';
    // Hide the File Viewer nav links when switching to editor
    var fvNav = document.getElementById('fv-td-nav');
    if (fvNav) fvNav.style.display = 'none';
    // Show/hide factory badge and adjust Save button
    updateFactoryBadge();
    // Fetch and show active template badge
    fetchActiveTemplate();
  }

  function closeTDEditor() {
    if (dirty && !confirm('You have unsaved changes. Discard them?')) return;
    var panel = document.getElementById('kit-panel');
    panel.classList.remove('td-editor-active');
    dirty = false;
    // Hide the File Viewer nav links
    var fvNav = document.getElementById('fv-td-nav');
    if (fvNav) fvNav.style.display = 'none';
  }

  /**
   * Switch from editor to JSON file viewer showing the actual file.
   * Fetches the raw file from the server so the user sees the real content.
   */
  function switchToJsonView() {
    var panel = document.getElementById('kit-panel');
    panel.classList.remove('td-editor-active');
    panel.classList.add('viewer-active');

    var fileName = currentFile ? currentFile.name : 'default.json';
    var filePath = currentFile ? currentFile.path : 'factory/trackdefaults';

    // Show the Editor/JSON nav links in File Viewer header
    var fvNav = document.getElementById('fv-td-nav');
    if (fvNav) fvNav.style.display = '';

    // Use the sample-manager openFileViewer to load the real file
    var fullPath = filePath + '/' + fileName;
    var body = document.getElementById('fv-body');
    body.innerHTML = '<sl-spinner></sl-spinner>';

    // Fetch the real file content from server
    fetch('/api/v2/storage?fetch=' + encodeURIComponent(fullPath))
      .then(function(r) { return r.text(); })
      .then(function(text) {
        var smState = window.TBD.sampleManager && window.TBD.sampleManager.state;
        if (smState) {
          smState.fileViewerOpen = true;
          smState.fileViewerData = {
            path: filePath,
            name: fileName,
            size: text.length,
            content: text,
            type: 'json'
          };
        }

        document.getElementById('fv-name').textContent = fileName;
        var iconDiv = document.getElementById('fv-icon');
        iconDiv.className = 'fv-tab-icon json';
        iconDiv.innerHTML = '<sl-icon name="file-earmark-code"></sl-icon>';
        document.getElementById('fv-lang').textContent = 'JSON';
        document.getElementById('fv-size').textContent = text.length + ' B';

        if (window.TBD.sampleManager && window.TBD.sampleManager.renderJson) {
          window.TBD.sampleManager.renderJson(body, text);
        } else {
          body.innerHTML = '<pre style="padding:1rem;color:#d4d4d4;">' + S.esc(text) + '</pre>';
        }
      })
      .catch(function() {
        body.innerHTML = '<div style="padding:1rem;color:#999;">Could not load file.</div>';
      });
  }

  /**
   * Open the track defaults editor for a specific file.
   * Called from: header button (default.json), file viewer toggle (any td file).
   */
  function openTrackDefaultsEditor(filePath, fileName) {
    currentFile = filePath ? { path: filePath, name: fileName || 'default.json' } : null;

    S.showLoading('Loading track setup…');
    var dataReady = S.data.loaded
      ? Promise.resolve()
      : S.loadSharedData();
    dataReady.then(function() {
      return loadTrackDefaults();
    }).then(function() {
      renderOverlayContent();
      S.hideLoading();
      showTDPanel();
    }).catch(function() {
      S.hideLoading();
      S.toast('Could not load track setup', 'danger', 4000);
    });
  }

  /**
   * Open from file viewer — parses the already-loaded JSON content.
   */
  function openFromFileViewer() {
    var smState = window.TBD.sampleManager && window.TBD.sampleManager.state;
    var fvData = smState && smState.fileViewerData;

    if (fvData && fvData.content) {
      currentFile = { path: fvData.path, name: fvData.name };
      try {
        var parsed = JSON.parse(fvData.content);
        trackDefaults = parsed && parsed.tracks ? parsed : { tracks: [] };
      } catch (e) {
        S.toast('Invalid JSON in file', 'danger', 4000);
        return;
      }
    } else {
      currentFile = null;
    }

    S.showLoading('Loading editor…');
    var dataReady = S.data.loaded
      ? Promise.resolve()
      : S.loadSharedData();
    dataReady.then(function() {
      return loadKitData();
    }).then(function() {
      renderOverlayContent();
      S.hideLoading();
      showTDPanel();
    }).catch(function() {
      S.hideLoading();
      S.toast('Could not load editor data', 'danger', 4000);
    });
  }

  // ─── Init ──────────────────────────────────────────────────

  function init() {
    var openBtn = document.getElementById('trackdefaults-btn');
    var closeBtn = document.getElementById('td-close-btn');

    // Header text links — TD editor side
    var tdLinkJson = document.getElementById('td-link-json');
    // Header text links — File Viewer side
    var fvLinkEditor = document.getElementById('fv-link-editor');

    if (openBtn) {
      openBtn.addEventListener('click', function() {
        openTrackDefaultsEditor('factory/trackdefaults', 'default.json');
      });
    }

    var saveAsBtn = document.getElementById('td-saveas-btn');
    if (saveAsBtn) {
      saveAsBtn.addEventListener('click', function() {
        saveAsDialog();
      });
    }

    var setActiveBtn = document.getElementById('td-setactive-btn');
    if (setActiveBtn) {
      setActiveBtn.addEventListener('click', function() {
        setAsBootDefault();
      });
    }

    var downloadBtn = document.getElementById('td-download-btn');
    if (downloadBtn) {
      downloadBtn.addEventListener('click', function() {
        if (!currentFile) return;
        var filePath = (currentFile.path ? currentFile.path + '/' : '') + currentFile.name;
        var url = '/api/v2/storage?download=' + encodeURIComponent(filePath);
        var a = document.createElement('a');
        a.href = url;
        a.download = currentFile.name;
        a.click();
      });
    }

    if (closeBtn) {
      closeBtn.addEventListener('click', function() {
        closeTDEditor();
      });
    }

    if (tdLinkJson) {
      tdLinkJson.addEventListener('click', function() {
        switchToJsonView();
      });
    }

    if (fvLinkEditor) {
      fvLinkEditor.addEventListener('click', function() {
        openFromFileViewer();
      });
    }
  }

  // ─── Export ────────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.trackDefaults = {
    init: init,
    openEditor: openTrackDefaultsEditor,
    openFromFileViewer: openFromFileViewer,
    isTrackDefaultsFile: function(path) {
      return /trackdefaults\//i.test(path || '');
    }
  };

})();

// ── preset-macro-app.js ───
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — App Controller
//
// Replaces app-persona.js. No persona switching.
// Handles: boot sequence, theme, connection monitor,
// sidebar tab switching, toolbar button context.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // Current sidebar tab: 'presets' or 'macros'
  var activeTab = 'presets';

  // ─── Theme (delegated to shared.js) ────────────────────────

  // ─── Settings ─────────────────────────────────────────────

  function setupSettings() {
    var btn = document.getElementById('config-btn');
    if (btn) {
      btn.addEventListener('click', function() {
        window.location.href = '/index.html?view=plugins&openConfig=1';
      });
    }
  }

  // ─── Quick Start Guide ───────────────────────────────────

  function setupQuickStart() {
    var dialog = document.getElementById('quickstart-dialog');
    var openBtn = document.getElementById('quickstart-btn');
    var closeBtn = document.getElementById('qs-close-btn');

    if (openBtn && dialog) {
      openBtn.addEventListener('click', function() {
        dialog.show();
      });
    }
    if (closeBtn && dialog) {
      closeBtn.addEventListener('click', function() {
        dialog.hide();
        localStorage.setItem('tbd-qs-seen', '1');
      });
    }

    // Auto-show on first visit
    if (!localStorage.getItem('tbd-qs-seen') && dialog) {
      setTimeout(function() { dialog.show(); }, 800);
    }
  }

  // ─── Connection Monitor (delegated to shared.js) ─────────

  // ─── Sidebar Tab Switching ───────────────────────────────

  function setupSidebarTabs() {
    document.querySelectorAll('.sidebar-tab').forEach(function(tab) {
      tab.addEventListener('click', function() {
        var tabName = tab.getAttribute('data-tab');
        switchTab(tabName);
      });
    });
  }

  function switchTab(tabName) {
    activeTab = tabName;

    // Update shared state so performer knows the mode
    if (S.setActiveTab) S.setActiveTab(tabName);

    // Update tab active states
    document.querySelectorAll('.sidebar-tab').forEach(function(tab) {
      tab.classList.toggle('active', tab.getAttribute('data-tab') === tabName);
    });

    // Update sidebar content visibility
    document.querySelectorAll('.sidebar-content').forEach(function(content) {
      content.classList.toggle('active', content.getAttribute('data-tab') === tabName);
    });

    // Show/hide center panel sub-tabs
    var subtabs = document.getElementById('center-subtabs');
    if (subtabs) {
      subtabs.classList.toggle('hidden', tabName !== 'macros');
    }

    // Update center panel content visibility
    updateCenterPanelVisibility();

    // Re-render knob controls (to update knob set dropdown visibility)
    if (window.TBD.performer && window.TBD.performer.renderKnobControls) {
      window.TBD.performer.renderKnobControls();
    }

    // When switching to macros, sync performer with designer's selected definition
    if (tabName === 'macros') {
      if (window.TBD.designer && window.TBD.designer.state && window.TBD.designer.state.editDef) {
        if (window.TBD.performer && window.TBD.performer.setMacroDef) {
          window.TBD.performer.setMacroDef(window.TBD.designer.state.editDef);
        }
      }
    }
  }

  // ─── Center Panel Sub-tabs (Macros mode) ─────────────────

  var activeSubTab = 'knob-preview';

  function setupCenterSubTabs() {
    document.querySelectorAll('.center-subtab').forEach(function(tab) {
      tab.addEventListener('click', function() {
        activeSubTab = tab.getAttribute('data-subtab');
        document.querySelectorAll('.center-subtab').forEach(function(t) {
          t.classList.toggle('active', t.getAttribute('data-subtab') === activeSubTab);
        });
        updateCenterPanelVisibility();

        // When switching to Knob Preview, re-render with the latest editDef
        if (activeSubTab === 'knob-preview' && activeTab === 'macros') {
          var D = window.TBD.designer;
          if (D && D.state && D.state.editDef) {
            if (window.TBD.performer && window.TBD.performer.setMacroDef) {
              window.TBD.performer.setMacroDef(D.state.editDef);
            }
          }
        }
      });
    });
  }

  function updateCenterPanelVisibility() {
    var knobControls = document.getElementById('knob-controls');
    var macroBuilder = document.getElementById('macro-builder-section');
    var defHeader = document.getElementById('macro-def-header');
    var knobPreviewExtras = document.getElementById('knob-preview-extras');

    if (activeTab === 'presets') {
      // Presets mode: always show knobs, hide macro builder and macro-specific panels
      if (knobControls) knobControls.classList.remove('hidden');
      if (macroBuilder) macroBuilder.classList.add('hidden');
      if (defHeader) defHeader.classList.add('hidden');
      if (knobPreviewExtras) knobPreviewExtras.classList.add('hidden');
    } else {
      // Macros mode: show based on selected sub-tab
      if (knobControls) knobControls.classList.toggle('hidden', activeSubTab !== 'knob-preview');
      if (macroBuilder) macroBuilder.classList.toggle('hidden', activeSubTab !== 'macro-builder');
      // Def header always visible in macros mode (renderDefHeader handles empty state)
      if (defHeader) defHeader.classList.remove('hidden');
      // Sound presets for def only visible in knob-preview sub-tab
      if (knobPreviewExtras) knobPreviewExtras.classList.toggle('hidden', activeSubTab !== 'knob-preview');
    }
  }

  // ─── Sidebar Preset Actions (Export/Import) ──────────────

  function setupPresetExportImport() {
    var exportBtn = document.getElementById('qa-export-presets');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        if (window.TBD.performer && window.TBD.performer.exportAllPresets) {
          window.TBD.performer.exportAllPresets();
        }
      });
    }

    var importBtn = document.getElementById('qa-import-presets');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        if (window.TBD.performer && window.TBD.performer.importPresetFile) {
          window.TBD.performer.importPresetFile();
        }
      });
    }
  }

  // ─── Keyboard Shortcuts ───────────────────────────────────

  function setupKeyboard() {
    document.addEventListener('keydown', function(e) {
      // Ctrl+1 / Ctrl+2 to switch sidebar tabs
      if (e.ctrlKey || e.metaKey) {
        if (e.key === '1') { e.preventDefault(); switchTab('presets'); }
        if (e.key === '2') { e.preventDefault(); switchTab('macros'); }
      }
    });
  }

  // ─── URL Parameter Handling (cross-links from FILE VIEWER) ─

  function handleUrlParams() {
    var params = new URLSearchParams(window.location.search);
    var tab = params.get('tab');
    var openDef = params.get('openDef');
    var openPreset = params.get('openPreset');

    if (tab === 'macros' || openDef) {
      switchTab('macros');
    } else if (tab === 'presets' || openPreset) {
      switchTab('presets');
    }

    if (openDef && S.data.macroDefs) {
      // Find the macro definition by ID
      var def = S.data.macroDefs.find(function(d) { return d.id === openDef; });
      if (def) {
        // Find a track that has the matching machine
        var targetTrack = S.data.tracks.find(function(t) {
          return (t.machines || []).indexOf(def.machine) >= 0;
        });
        if (targetTrack) {
          S.selectTrack(targetTrack.index);
        }
        // Give UI time to settle after track selection, then select the definition
        setTimeout(function() {
          if (window.TBD.designer && window.TBD.designer.selectMacroDefinition) {
            window.TBD.designer.selectMacroDefinition(openDef);
          }
        }, 300);
      }
    }

    if (openPreset && S.data.soundPresets) {
      // Find preset by ID and select its macro to show it in context
      var preset = S.data.soundPresets.find(function(p) { return p.id === openPreset; });
      if (preset && preset.macro) {
        var pDef = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
        if (pDef) {
          var targetTrack = S.data.tracks.find(function(t) {
            return (t.machines || []).indexOf(pDef.machine) >= 0;
          });
          if (targetTrack) {
            S.selectTrack(targetTrack.index);
          }
        }
        // Give UI time to settle, then select the macro + scroll to preset
        setTimeout(function() {
          if (window.TBD.designer && window.TBD.designer.selectMacroDefinition && preset.macro) {
            window.TBD.designer.selectMacroDefinition(preset.macro);
          }
          // Highlight the target preset card after a short additional delay
          setTimeout(function() {
            var presetCard = document.querySelector('.sp-card[data-preset-id="' + openPreset + '"]');
            if (presetCard) {
              presetCard.scrollIntoView({ behavior: 'smooth', block: 'center' });
              presetCard.style.outline = '2px solid var(--sl-color-primary-500)';
              setTimeout(function() { presetCard.style.outline = ''; }, 2000);
            }
          }, 200);
        }, 300);
      }
    }

    // Clean URL params without triggering a reload
    if (params.toString()) {
      window.history.replaceState({}, '', window.location.pathname);
    }
  }

  // ─── Boot Sequence ────────────────────────────────────────

  function boot() {
    console.log('[TBD-16] Booting unified UI…');

    S.setupThemeToggle('theme-toggle');
    setupSettings();
    setupQuickStart();
    S.startConnectionMonitor();
    setupSidebarTabs();
    setupCenterSubTabs();
    setupPresetExportImport();
    setupKeyboard();

    // Factory lock button in footer
    if (window.TBD.factory && window.TBD.factory.setupFooterLock) {
      window.TBD.factory.setupFooterLock();
    }

    // Load shared data, then init modules
    S.loadSharedData().then(function() {
      console.log('[TBD-16] Shared data loaded');

      // Render track overview
      S.renderTrackOverview();
      S.setupTrackOverviewEvents();

      // Init modules
      if (window.TBD.performer && window.TBD.performer.init) {
        window.TBD.performer.init();
      }
      if (window.TBD.designer && window.TBD.designer.init) {
        window.TBD.designer.init();
      }
      if (window.TBD.trackDefaults && window.TBD.trackDefaults.init) {
        window.TBD.trackDefaults.init();
      }

      // Auto-select first track
      if (S.data.tracks.length > 0) {
        S.selectTrack(S.data.tracks[0].index);
      }

      // Handle URL params (cross-links from FILE VIEWER)
      handleUrlParams();

      S.hideLoading();
      console.log('[TBD-16] Boot complete');
    }).catch(function(err) {
      console.error('[TBD-16] Boot failed:', err);
      S.toast('Boot failed: ' + err.message, 'danger', 5000);
      S.hideLoading();
    });
  }

  // ─── Start on DOM ready ──────────────────────────────────

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', boot);
  } else {
    boot();
  }

})();

