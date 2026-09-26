/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D Protocol.
 *
 * D2D Protocol is a new transport protocol which is created to transport data
 * over high-speed half-duplex channels. The protocol is designed to keep
 * backward traffic rate low and tuneable because big amount of acknowledgements
 * can limit the maximum bandwidth available in the half-duplex channels. You
 * can think about it as "simplified TCP with constant-rate ACKs".
 *
 * Right now only UDP is supported as underlying transport.
 */

#ifndef D2D_H
#define D2D_H

#include <linux/cred.h>
#include <linux/net.h>
#include <linux/types.h>

/**
 * struct d2d_protocol - the full D2D Protocol structure
 *
 * One instance of this structure encapsulates all the protocol internals. This
 * structure is so huge so it has been decided to hide the internals using
 * opaque type with forward declaration.
 */
struct d2d_protocol;

/**
 * enum d2d_option - available options for D2D Protocol
 *
 * @D2D_OPT_RCVTIMEO: Specify the receiving timeout for recv functions until
 *		      reporting an error. The argument is an int (milliseconds).
 *		      If a recv function blocks for this period of time, and
 *		      data has been received, the return value of that function
 *		      will be the amount of data transferred; if no data has
 *		      been transferred and the timeout has been reached then
 *		      -EAGAIN is returned. If the timeout is set to zero (the
 *		      default) then the operation will never timeout. If the
 *		      timeout is less than zero then d2d_set_opt function will
 *		      return -EINVAL.
 */
enum d2d_option {
	D2D_OPT_RCVTIMEO = 0,
	D2D_OPT_TXCPUMASK,
};

/**
 * enum d2d_recv_flags - available flags for D2D receiving functions
 *
 * @D2D_RECV_FLAG_WAITALL: This flag requests that the recv function block until
 *                         the full request is satisfied.
 */
enum d2d_recv_flags {
	D2D_RECV_FLAG_WAITALL = 0x1,
};

/**
 * enum d2d_send_flags - available flags for D2D sending functions
 *
 * @D2D_SEND_FLAG_KEY_UPDATE: This flag is used to instruct the protocol that
 *                            the following send call contains the app-level
 *                            *update key* message. This message is handled by
 *                            the procotol with special care: on the consequent
 *                            d2d_crypto_tx_lock() call the protocol will wait
 *                            until this message is acknowledged. Note that you
 *                            cannot use this flag until you complete full key
 *                            update cycle, if you try to do that, send()
 *                            function will return -EALREADY error.
 */
enum d2d_send_flags {
	D2D_SEND_FLAG_KEY_UPDATE = 0x1,
};

/**
 * d2d_set_opt - set options for D2D Protocol
 * @proto: the protocol instance to set options for
 * @optname: name of option
 * @optval: pointer to option value
 * @optlen: size of option value
 *
 * Returns 0 on success and negative integer on error.
 */
int d2d_set_opt(struct d2d_protocol *proto, int optname, const void *optval,
		size_t optlen);

/**
 * d2d_get_opt - get value of D2D Protocol option
 * @proto: the protocol instance
 * @optname: name of option
 * @optval: pointer to store obtained option value
 *
 * Returns 0 on success and negative integer on error.
 */
int d2d_get_opt(struct d2d_protocol *proto, int optname, void *optval);

/**
 * struct d2d_security - D2D Protocol's security interface for the upper layers
 *
 * Crypto handlers have to be implemented on the calling level. They will be
 * called inside D2D protocol to encrypt data right before sending message to
 * underlying transport and to decrypt incoming messages right after reception.
 *
 * @cred: credentials to use for D2DP threads instead of default creds
 * @cr_overhead: amount of additional bytes required for the encryption data
 * @key_info: pointer to custom data containing keys
 * @encrypt: encryption virtual function
 * @decrypt: decryption virtual function
 */
struct d2d_security {
	const struct cred *cred;
	size_t cr_overhead;
	void *key_info;
	int (*encrypt)(struct d2d_security *self, void *src, size_t src_len,
		       void *dst, size_t dst_len);
	int (*decrypt)(struct d2d_security *self, void *src, size_t src_len,
		       void *dst, size_t dst_len);
};

/**
 * d2d_protocol_create - create a new instance of D2D Protocol
 * @sock: valid UDP socket in connected state
 * @flags: integer flags for the protocol
 * @crypto: crypto interface implementation
 *
 * This call will create a new instance of D2D Protocol. The @sock must be a UDP
 * socket in connected state. The @flags argument is reserved for the future now
 * it is ignored by the protocol. The @security implementation is required if
 * you want to protect the protocol, otherwise you can pass NULL here and the
 * protocol will not use any protection at all. The function returns a pointer
 * to the created protocol structure or NULL on error.
 *
 * NOTE: the @sock and @crypto (if not NULL) must be initialized before calling
 * d2d_protocol_create() and cannot be freed before d2d_protocol_close()
 */
struct d2d_protocol *d2d_protocol_create(struct socket *sock, uint32_t flags,
					 struct d2d_security *security);

/**
 * d2d_protocol_close - destroy the instance of D2D Protocol
 * @proto: the protocol instance to close
 *
 * Returns 0 on success and negative integer on error. This function may be
 * blocked waiting for acknowledgements for sent packets.
 *
 * NOTE: ensure that on the moment of protocol closing there are no other API
 * callers pending on d2d_send()/d2d_sendiov() or d2d_recv()/d2d_recviov().
 * Otherwise random use-after-free errors can happen.
 */
int d2d_protocol_close(struct d2d_protocol *proto);

/**
 * d2d_send - send buffer over D2D Protocol
 * @proto: D2D Protocol instance
 * @data: pointer to the data to send
 * @size: data length
 * @flags: integer flags for sending function
 *
 * Returns the number of bytes sent or negative error. This function may be
 * blocked if the TX buffer is full.
 */
ssize_t d2d_send(struct d2d_protocol *proto, const void *data, size_t size,
		 uint32_t flags);

/**
 * d2d_recv - receive data buffer from D2D Protocol
 * @proto: D2D Protocol instance
 * @data: pointer to the buffer to write received data
 * @size: buffer length
 * @flags: integer flags for receiving function
 *
 * Blocks until RX buffer collects some contiguos stream data or timeout
 * has been reached (D2D_OPT_TIMEOUT). Returns the number of bytes received
 * or negative error.
 */
ssize_t d2d_recv(struct d2d_protocol *proto, void *data, size_t size,
		 uint32_t flags);

/**
 * d2d_sendiov - send iov over D2D Protocol
 * @proto: D2D Protocol instance
 * @iov: pointer to the kvec structures containing data to send
 * @iovlen: length of iov array
 * @to_send: requested amount of bytes to send
 * @flags: integer flags for sending function
 *
 * Returns the number of bytes sent or negative error. This function may be
 * blocked if the TX buffer is full.
 */
ssize_t d2d_sendiov(struct d2d_protocol *proto, const struct kvec *iov,
		    size_t iovlen, size_t to_send, uint32_t flags);

/**
 * d2d_recviov - receive iov from D2D Protocol
 * @proto: D2D Protocol instance
 * @iov: pointer to the kvec structures to store received data
 * @iovlen: length of iov array
 * @to_recv: requested amount of bytes to receive
 * @flags: integer flags for receiving function
 *
 * Blocks until RX buffer collects some contiguos stream data or timeout
 * has been reached (D2D_OPT_TIMEOUT). Returns the number of bytes received
 * or negative error.
 */
ssize_t d2d_recviov(struct d2d_protocol *proto, struct kvec *iov,
		    size_t iovlen, size_t to_recv, uint32_t flags);

/*
 * A short guideline for updating crypto keys.
 *
 * On TX side make the following sequence of actions:
 * 1. send app-level message with key update request with KEY_UPDATE send flag
 * 2. call d2d_crypto_tx_lock(). It blocks and it can fail
 * 3. update TX keys in `struct d2d_crypto`
 * 4. call d2d_crypto_tx_unlock().
 *
 * Note that 2nd action can fail so the caller MUST check the return value.
 *
 * On RX side use the following sequence:
 * 1. recv app-level message with key update request
 * 2. call d2d_crypto_rx_lock(). It blocks but cannot fail
 * 3. update RX keys in `struct d2d_crypto`
 * 4. call d2d_crypto_rx_unlock()
 */

/**
 * d2d_crypto_tx_lock - prepare the protocol instance for safe TX key update
 * @proto: the protocol instance
 *
 * This function will wait until the other side of the D2DP connection
 * acknowledges the key update request (which was sent with KEY_UPDATE flag).
 * When the function returns with success, it is safe to update TX keys in
 * `d2d_crypto`. Otherwise, the locking failed and updating TX keys is unsafe.
 *
 * Returns 0 on success or negative error.
 */
int __must_check d2d_crypto_tx_lock(struct d2d_protocol *proto);

/**
 * d2d_crypto_tx_unlock - restore the protocol operation after TX key update
 * @proto: the protocol instance
 *
 * This function restores the normal mode of protocol operation
 */
void d2d_crypto_tx_unlock(struct d2d_protocol *proto);

/**
 * d2d_crypto_rx_lock - prepare the protocol instance for safe RX key update
 * @proto: the protocol instance
 *
 * When the function returns, it is safe to update RX keys in `d2d_crypto`
 */
void d2d_crypto_rx_lock(struct d2d_protocol *proto);

/**
 * d2d_crypto_rx_unlock - restore the protocol operation after RX key update
 * @proto: the protocol instance
 *
 * This function restores the normal mode of protocol operation
 */
void d2d_crypto_rx_unlock(struct d2d_protocol *proto);

#endif /* D2D_H */
