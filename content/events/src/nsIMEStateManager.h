/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsIMEStateManager_h__
#define nsIMEStateManager_h__

#include "nscore.h"
#include "nsEvent.h"
#include "nsIWidget.h"

class nsDispatchingCallback;
class nsIContent;
class nsIDOMMouseEvent;
class nsINode;
class nsPIDOMWindow;
class nsPresContext;
class nsTextStateManager;
class nsISelection;

namespace mozilla {
class TextCompositionArray;
} // namespace mozilla

/*
 * IME state manager
 */

class nsIMEStateManager
{
protected:
  typedef mozilla::widget::IMEState IMEState;
  typedef mozilla::widget::InputContext InputContext;
  typedef mozilla::widget::InputContextAction InputContextAction;

public:
  static void Shutdown();

  static nsresult OnDestroyPresContext(nsPresContext* aPresContext);
  static nsresult OnRemoveContent(nsPresContext* aPresContext,
                                  nsIContent* aContent);
  /**
   * OnChangeFocus() should be called when focused content is changed or
   * IME enabled state is changed.  If focus isn't actually changed and IME
   * enabled state isn't changed, this will do nothing.
   */
  static nsresult OnChangeFocus(nsPresContext* aPresContext,
                                nsIContent* aContent,
                                InputContextAction::Cause aCause);
  static void OnInstalledMenuKeyboardListener(bool aInstalling);

  // These two methods manage focus and selection/text observers.
  // They are separate from OnChangeFocus above because this offers finer
  // control compared to having the two methods incorporated into OnChangeFocus

  // OnTextStateBlur should be called *before* NS_BLUR_CONTENT fires
  // aPresContext is the nsPresContext receiving focus (not lost focus)
  // aContent is the nsIContent receiving focus (not lost focus)
  // aPresContext and/or aContent may be null
  static nsresult OnTextStateBlur(nsPresContext* aPresContext,
                                  nsIContent* aContent);
  // OnTextStateFocus should be called *after* NS_FOCUS_CONTENT fires
  // aPresContext is the nsPresContext receiving focus
  // aContent is the nsIContent receiving focus
  static nsresult OnTextStateFocus(nsPresContext* aPresContext,
                                   nsIContent* aContent);
  // Get the focused editor's selection and root
  static nsresult GetFocusSelectionAndRoot(nsISelection** aSel,
                                           nsIContent** aRoot);
  // This method updates the current IME state.  However, if the enabled state
  // isn't changed by the new state, this method does nothing.
  // Note that this method changes the IME state of the active element in the
  // widget.  So, the caller must have focus.
  static void UpdateIMEState(const IMEState &aNewIMEState,
                             nsIContent* aContent);

  // This method is called when user clicked in an editor.
  // aContent must be:
  //   If the editor is for <input> or <textarea>, the element.
  //   If the editor is for contenteditable, the active editinghost.
  //   If the editor is for designMode, NULL.
  static void OnClickInEditor(nsPresContext* aPresContext,
                              nsIContent* aContent,
                              nsIDOMMouseEvent* aMouseEvent);

  /**
   * All DOM composition events and DOM text events must be dispatched via
   * DispatchCompositionEvent() for storing the composition target
   * and ensuring a set of composition events must be fired the stored target.
   * If the stored composition event target is destroying, this removes the
   * stored composition automatically.
   */
  static void DispatchCompositionEvent(nsINode* aEventTargetNode,
                                       nsPresContext* aPresContext,
                                       nsEvent* aEvent,
                                       nsEventStatus* aStatus,
                                       nsDispatchingCallback* aCallBack);

  /**
   * Send a notification to IME.  It depends on the IME or platform spec what
   * will occur (or not occur).
   */
  static nsresult NotifyIME(mozilla::widget::NotificationToIME aNotification,
                            nsIWidget* aWidget);
  static nsresult NotifyIME(mozilla::widget::NotificationToIME aNotification,
                            nsPresContext* aPresContext);

protected:
  static nsresult OnChangeFocusInternal(nsPresContext* aPresContext,
                                        nsIContent* aContent,
                                        InputContextAction aAction);
  static void SetIMEState(const IMEState &aState,
                          nsIContent* aContent,
                          nsIWidget* aWidget,
                          InputContextAction aAction);
  static IMEState GetNewIMEState(nsPresContext* aPresContext,
                                 nsIContent* aContent);

  static void EnsureTextCompositionArray();

  static nsIContent*    sContent;
  static nsPresContext* sPresContext;
  static bool           sInstalledMenuKeyboardListener;
  static bool           sInSecureInputMode;

  static nsTextStateManager* sTextStateObserver;

  // All active compositions in the process are stored by this array.
  // When you get an item of this array and use it, please be careful.
  // The instances in this array can be destroyed automatically if you do
  // something to cause committing or canceling the composition.
  static mozilla::TextCompositionArray* sTextCompositions;
};

#endif // nsIMEStateManager_h__
